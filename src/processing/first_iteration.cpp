#include <limits>
#include <future>
#include <vector>
#include <cmath>
#include <iostream>

#include "../utils/utils.h"
#include "../utils/singleton.h"
#include "../utils/resource_manager.h"
#include "first_iteration.h"

namespace kiv_ppr
{
    CFirst_Iteration::CFirst_Iteration(CFile_Reader<double>* file)
        : m_file(file),
          m_values{},
          m_worker_means{}
    {

    }

    typename CFirst_Iteration::TValues CFirst_Iteration::Get_Values() const noexcept
    {
        return m_values;
    }

    int CFirst_Iteration::Run(config::TThread_Params* thread_config)
    {
        m_file->Seek_Beg();

        std::vector<std::future<int>> workers(thread_config->number_of_threads);
        //CWatchdog watchdog(thread_config->watchdog_expiration_sec);
        for (auto& worker : workers)
        {
            worker = std::async(std::launch::async, &CFirst_Iteration::Worker, this, thread_config);
        }

        int return_values = 0;
        for (auto& worker : workers)
        {
            return_values += worker.get();
        }
        //watchdog.Stop();

        for (const auto& [mean, count] : m_worker_means)
        {
            m_values.mean += mean * (static_cast<double>(count) / static_cast<double>(m_values.count));
        }

        if (return_values != 0 /* || watchdog.Get_Counter_Value() != m_file->Get_Number_Of_Elements() */)
        {
            return 1;
        }
        return 0;
    }

    void CFirst_Iteration::Report_Worker_Results(TValues values)
    {
        const std::lock_guard<std::mutex> lock(m_mtx);

        m_values.min = std::min(m_values.min, values.min);
        m_values.max = std::max(m_values.max, values.max);
        m_values.count += values.count;

        m_worker_means.emplace_back(values.mean, values.count);

        if (m_values.all_ints && !values.all_ints)
        {
            m_values.all_ints = false;
        }
    }

    inline CFirst_Iteration::TValues CFirst_Iteration::Agregate_Results_From_GPU(
                const std::vector<double>& out_min,
                const std::vector<double>& out_max,
                const std::vector<double>& out_mean,
                const std::vector<double>& out_count,
                size_t total_count)
    {
        TValues values{};
        size_t size = out_min.size();
        size_t count = 0;

        for (size_t i = 0; i < size; ++i)
        {
            if (utils::Is_Valid_Double(out_min[i]))
            {
                values.min = std::min(values.min, out_min[i]);
            }
            if (utils::Is_Valid_Double(out_max[i]))
            {
                values.max = std::max(values.max, out_max[i]);
            }
            if (utils::Is_Valid_Double(out_mean[i]))
            {
                values.mean += out_mean[i] * (out_count[i] / static_cast<double>(total_count));
            }
        }
        return values;
    }

    CFirst_Iteration::TValues CFirst_Iteration::Process_Data_Block_On_CPU(CFile_Reader<double>::TData_Block& data_block, size_t offset)
    {
        TValues values{};
        size_t count = 0;
        double delta;

        for (size_t i = offset; i < data_block.count; ++i)
        {
            if (utils::Is_Valid_Double(data_block.data[i]))
            {
                double value = data_block.data[i] / config::processing::Scale_Factor;

                values.min = std::min(values.min, value);
                values.max = std::max(values.max, value);

                ++values.count;
                delta = value - values.mean;
                values.mean += delta / values.count;

                if (values.all_ints && (std::floor(data_block.data[i]) != std::ceil(data_block.data[i])))
                {
                    values.all_ints = false;
                }
            }
        }
        return values;
    }

    CFirst_Iteration::TOpenCL_Report CFirst_Iteration::Execute_OpenCL(kernels::TOpenCL_Settings& opencl, CFile_Reader<double>::TData_Block& data_block)
    {
        //std::cout << "CL_DEVICE_LOCAL_MEM_SIZE = " << opencl.local_mem_size << std::endl;
        //std::cout << "Local mem to be used = " << (opencl.work_group_size * kernels::First_Iteration_Get_Size_Of_Local_Params) << std::endl;

        // TODO think of a better solution
        while (opencl.work_group_size * kernels::First_Iteration_Get_Size_Of_Local_Params > opencl.local_mem_size)
        {
            opencl.work_group_size /= 2;
        }
        if (0 == opencl.work_group_size)
        {
            // TODO print out an error message
            std::cout << "";
            std::exit(9);
        }

        const auto work_groups_count = data_block.count / opencl.work_group_size;
        const size_t count = data_block.count - (data_block.count % opencl.work_group_size);

        if (0 == work_groups_count)
        {
            return { false, false, 0, {} };
        }

        cl::Buffer data_buff(opencl.context, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_USE_HOST_PTR, count * sizeof(double), data_block.data.get());
        cl::Buffer out_mean_buff(opencl.context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, work_groups_count * sizeof(double));
        cl::Buffer out_min_buff(opencl.context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, work_groups_count * sizeof(double));
        cl::Buffer out_max_buff(opencl.context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, work_groups_count * sizeof(double));
        cl::Buffer out_count_buff(opencl.context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, work_groups_count * sizeof(double));
        cl::Buffer out_all_ints_buff(opencl.context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, work_groups_count * sizeof(int));

        try
        {
            opencl.kernel.setArg(0, data_buff);
            opencl.kernel.setArg(1, opencl.work_group_size * sizeof(double), nullptr);
            opencl.kernel.setArg(2, out_mean_buff);
            opencl.kernel.setArg(3, opencl.work_group_size * sizeof(double), nullptr);
            opencl.kernel.setArg(4, out_min_buff);
            opencl.kernel.setArg(5, opencl.work_group_size * sizeof(double), nullptr);
            opencl.kernel.setArg(6, out_max_buff);
            opencl.kernel.setArg(7, opencl.work_group_size * sizeof(int), nullptr);
            opencl.kernel.setArg(8, out_all_ints_buff);
            opencl.kernel.setArg(9, opencl.work_group_size * sizeof(double), nullptr);
            opencl.kernel.setArg(10, out_count_buff);
        }
        catch (const cl::Error& e)
        {
            kernels::Print_OpenCL_Error(e, *opencl.device);
            std::exit(7);
        }

        std::vector<double> out_min(work_groups_count);
        std::vector<double> out_max(work_groups_count);
        std::vector<double> out_mean(work_groups_count);
        std::vector<double> out_count(work_groups_count);
        std::vector<int> out_all_ints(work_groups_count);

        cl::CommandQueue cmd_queue(opencl.context, *opencl.device);

        try
        {
            cmd_queue.enqueueNDRangeKernel(opencl.kernel, cl::NullRange, cl::NDRange(count), cl::NDRange(opencl.work_group_size));
        }
        catch (const cl::Error& e)
        {
            kernels::Print_OpenCL_Error(e, *opencl.device);
            std::exit(8);
        }

        try
        {
            cmd_queue.enqueueReadBuffer(out_mean_buff, CL_TRUE, 0, out_mean.size() * sizeof(double), out_mean.data());
            cmd_queue.enqueueReadBuffer(out_min_buff, CL_TRUE, 0, out_min.size() * sizeof(double), out_min.data());
            cmd_queue.enqueueReadBuffer(out_max_buff, CL_TRUE, 0, out_max.size() * sizeof(double), out_max.data());
            cmd_queue.enqueueReadBuffer(out_count_buff, CL_TRUE, 0, out_count.size() * sizeof(double), out_count.data());
            cmd_queue.enqueueReadBuffer(out_all_ints_buff, CL_TRUE, 0, out_all_ints.size() * sizeof(unsigned long), out_all_ints.data());
        }
        catch (const cl::Error& e)
        {
            kernels::Print_OpenCL_Error(e, *opencl.device);
            std::exit(9);
        }

        size_t number_of_valid_doubles = 0;
        bool all_ints = true;
        for (size_t i = 0; i < work_groups_count; ++i)
        {
            number_of_valid_doubles += static_cast<size_t>(out_count[i]);
            all_ints = all_ints && out_all_ints[i];
        }

        const auto aggregated_vals = Agregate_Results_From_GPU(out_min, out_max, out_mean, out_count, number_of_valid_doubles);

        return { true, count == data_block.count, {
            aggregated_vals.min,
            aggregated_vals.max,
            aggregated_vals.mean,
            number_of_valid_doubles,
            all_ints
        } };
    }

    int CFirst_Iteration::Worker(config::TThread_Params* thread_config)
    {
        TValues local_values{};

        auto resource_manager = Singleton<CResource_Manager>::Get_Instance();
        const cl::Device* device = nullptr;
        kernels::TOpenCL_Settings opencl;
        bool use_cpu = false;

        const auto run_type = resource_manager->Get_Run_Type();
        if (run_type == CArg_Parser::NRun_Type::SMP)
        {
            use_cpu = true;
        }
        if (run_type == CArg_Parser::NRun_Type::All || run_type == CArg_Parser::NRun_Type::OpenCL_Devs)
        {
            device = resource_manager->Get_Avaliable_Device();
            if (run_type == CArg_Parser::NRun_Type::OpenCL_Devs && nullptr == device)
            {
                return 0;
            }
            else if (nullptr == device)
            {
                use_cpu = true;
            }
        }
        if (nullptr != device)
        {
            opencl = kernels::Init_OpenCL(device, kernels::First_Iteration_Kernel, kernels::First_Iteration_Kernel_Name);
        }

        while (true)
        {
            auto data_block = m_file->Read_Data(thread_config->number_of_elements_per_file_read);
            switch (data_block.status)
            {
                case kiv_ppr::CFile_Reader<double>::NRead_Status::OK:
                    if (use_cpu)
                    {
                        Execute_On_CPU(local_values, data_block);
                    }
                    else
                    {
                        Execute_On_GPU(local_values, data_block, opencl);
                    }
                    break;

                case CFile_Reader<double>::NRead_Status::EOF_:
                    Report_Worker_Results(local_values);
                    if (!use_cpu)
                    {
                        resource_manager->Release_Device(device);
                    }
                    return 0;

                case CFile_Reader<double>::NRead_Status::Error: [[fallthrough]];
                default:
                    if (!use_cpu)
                    {
                        resource_manager->Release_Device(device);
                    }
                    return 1;
            }
        }
    }

    void CFirst_Iteration::Execute_On_CPU(TValues& local_values, const CFile_Reader<double>::TData_Block& data_block)
    {
        double delta;
        for (auto i = 0; i < data_block.count; ++i)
        {
            double value = data_block.data[i];
            if (utils::Is_Valid_Double(value))
            {
                if (local_values.all_ints && (std::floor(value) != std::ceil(value)))
                {
                    local_values.all_ints = false;
                }

                value /= config::processing::Scale_Factor;

                local_values.min = std::min(local_values.min, value);
                local_values.max = std::max(local_values.max, value);

                ++local_values.count;
                delta = value - local_values.mean;
                local_values.mean += delta / static_cast<double>(local_values.count);
            }
        }
    }

    void CFirst_Iteration::Execute_On_GPU(TValues& local_values, CFile_Reader<double>::TData_Block& data_block, kernels::TOpenCL_Settings& opencl)
    {
        const auto opencl_report = Execute_OpenCL(opencl, data_block);
        TValues gpu_values = opencl_report.values;

        if (!opencl_report.success)
        {
            gpu_values = Process_Data_Block_On_CPU(data_block, 0);
        }
        else if (!opencl_report.all_processed)
        {
            const size_t offset = data_block.count - (data_block.count % opencl.work_group_size);
            const auto cpu_values = Process_Data_Block_On_CPU(data_block, offset);
            Merge_Values(gpu_values, cpu_values);
        }
        Merge_Values(local_values, gpu_values);
    }

    void CFirst_Iteration::Merge_Values(TValues& dest, const TValues& src)
    {
        const size_t total_count = dest.count + src.count;

        dest.min = std::min(dest.min, src.min);
        dest.max = std::max(dest.max, src.max);

        dest.mean *= (static_cast<double>(dest.count) / static_cast<double>(total_count));
        dest.mean += src.mean * (static_cast<double>(src.count) / static_cast<double>(total_count));

        dest.all_ints = dest.all_ints && src.all_ints;
        dest.count = total_count;
    }
}