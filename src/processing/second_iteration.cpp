#include <future>
#include <cmath>

#include "second_iteration.h"
#include "../utils/utils.h"
#include "../utils/singleton.h"
#include "../utils/resource_guard.h"
#include "../utils/resource_manager.h"

namespace kiv_ppr
{
    CSecond_Iteration::CSecond_Iteration(CFile_Reader<double>* file,
                                         typename CFirst_Iteration::TValues* basic_values)
        : m_file(file),
          m_basic_values(basic_values),
          m_values{},
          m_histogram_params{}
    {
        if (m_basic_values->min >= 0)
        {
            Scale_Up_Basic_Values(m_basic_values);
        }
        m_histogram_params = {
            m_basic_values->min,
            m_basic_values->max,
            Calculate_Number_Of_Intervals(m_basic_values->count)
        };
        m_values.histogram = std::make_shared<CHistogram>(m_histogram_params);
    }

    void CSecond_Iteration::Scale_Up_Basic_Values(typename CFirst_Iteration::TValues* basic_values)
    {
        basic_values->min *= config::processing::Scale_Factor;
        basic_values->max *= config::processing::Scale_Factor;
        basic_values->mean *= config::processing::Scale_Factor;
    }

    typename CSecond_Iteration::TValues CSecond_Iteration::Get_Values() const noexcept
    {
        return m_values;
    }

    int CSecond_Iteration::Run(config::TThread_Params* thread_config)
    {
        m_file->Seek_Beg();

        std::vector<std::future<int>> workers(thread_config->number_of_threads);
        CWatchdog watchdog(thread_config->watchdog_expiration_sec);

        for (auto& worker : workers)
        {
            worker = std::async(std::launch::async, &CSecond_Iteration::Worker, this, thread_config, &watchdog);
        }

        int return_values = 0;
        for (auto& worker : workers)
        {
            return_values += worker.get();
        }
        watchdog.Stop();

        m_values.sd = std::sqrt(m_values.var);
        if (return_values != 0 || watchdog.Get_Counter_Value() != m_file->Get_Number_Of_Elements())
        {
            return 1;
        }
        return 0;
    }

    void CSecond_Iteration::Report_Worker_Results(const TValues& values)
    {
        const std::lock_guard<std::mutex> lock(m_mtx);
        m_values.var += values.var;
        m_values.histogram->operator+=(*values.histogram);
    }

    CSecond_Iteration::TOpenCL_Report CSecond_Iteration::Execute_OpenCL(kernels::TOpenCL_Settings& opencl, CFile_Reader<double>::TData_Block& data_block, TValues& local_values)
    {
        const auto work_groups_count = data_block.count / opencl.work_group_size;
        const size_t count = data_block.count - (data_block.count % opencl.work_group_size);

        if (0 == work_groups_count)
        {
            return { false, false };
        }

        auto& intervals = local_values.histogram->Get_Intervals();
        double interval_size = local_values.histogram->Get_Interval_Size();

        cl::Buffer data_buff(opencl.context, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_USE_HOST_PTR, count * sizeof(double), data_block.data.get());
        cl::Buffer out_var_buff(opencl.context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, work_groups_count * sizeof(double));
        cl::Buffer histogram_buff(opencl.context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, 2 * intervals.size() * sizeof(cl_uint));

        try
        {
            opencl.kernel.setArg(0, data_buff);
            opencl.kernel.setArg(1, opencl.work_group_size * sizeof(double), nullptr);
            opencl.kernel.setArg(2, out_var_buff);
            opencl.kernel.setArg(3, histogram_buff);
            opencl.kernel.setArg(4, sizeof(double), &m_basic_values->mean);
            opencl.kernel.setArg(5, sizeof(cl_ulong), &m_basic_values->count);
            opencl.kernel.setArg(6, sizeof(double), &m_basic_values->min);
            opencl.kernel.setArg(7, sizeof(double), &interval_size);
        }
        catch (const cl::Error& e)
        {
            kernels::Print_OpenCL_Error(e, *opencl.device);
            std::exit(11);
        }

        std::vector<double> out_var(work_groups_count);
        std::vector<cl_uint> out_hisogram(2 * intervals.size());

        cl::CommandQueue cmd_queue(opencl.context, *opencl.device);

        try
        {
            cmd_queue.enqueueNDRangeKernel(opencl.kernel, cl::NullRange, cl::NDRange(count), cl::NDRange(opencl.work_group_size));
        }
        catch (const cl::Error& e)
        {
            kernels::Print_OpenCL_Error(e, *opencl.device);
            std::exit(12);
        }

        try
        {
            cmd_queue.enqueueReadBuffer(out_var_buff, CL_TRUE, 0, out_var.size() * sizeof(double), out_var.data());
            cmd_queue.enqueueReadBuffer(histogram_buff, CL_TRUE, 0, out_hisogram.size() * sizeof(cl_uint), out_hisogram.data());
        }
        catch (const cl::Error& e)
        {
            kernels::Print_OpenCL_Error(e, *opencl.device);
            std::exit(13);
        }

        size_t size = intervals.size();
        for (size_t i = 0; i < size; ++i)
        {
            const size_t value = static_cast<size_t>(out_hisogram[2 * i]) + static_cast<size_t>(out_hisogram[2 * i + 1]) * sizeof(cl_uint);
            if (false == local_values.histogram->Add(i, value))
            {
                std::cout << "Failed pathing (updating) the histogram" << std::endl;
                std::exit(14);
            }
        }

        double gpu_var = 0.0;
        for (const auto& value : out_var)
        {
            gpu_var += value;
        }
        local_values.var += gpu_var;

        return { true, count == data_block.count };
    }

    int CSecond_Iteration::Worker(config::TThread_Params* thread_config, CWatchdog* watchdog)
    {
        TValues local_values{};
        local_values.histogram = std::make_shared<CHistogram>(m_histogram_params);

        auto resource_manager = Singleton<CResource_Manager>::Get_Instance();
        const cl::Device* device = nullptr;
        kernels::TOpenCL_Settings opencl;
        CResource_Guard opencl_device_guard;
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
            opencl = kernels::Init_OpenCL(device, kernels::Second_Iteration_Kernel, kernels::Second_Iteration_Kernel_Name);
            kernels::Adjust_Work_Group_Size(opencl, kernels::Second_Iteration_Get_Size_Of_Local_Params);
            opencl_device_guard.Set_Device(device);
        }

        watchdog->Start();

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
                    watchdog->Kick(data_block.count);
                    break;

                case CFile_Reader<double>::NRead_Status::EOF_:
                    Report_Worker_Results(local_values);
                    return 0;

                case CFile_Reader<double>::NRead_Status::Error: [[fallthrough]];
                default:
                    return 1;
            }
        }
    }

    size_t CSecond_Iteration::Calculate_Number_Of_Intervals(size_t n)
    {
        // https://onlinelibrary.wiley.com/doi/full/10.1002/1097-0320%2820011001%2945%3A2%3C141%3A%3AAID-CYTO1156%3E3.0.CO%3B2-M#bib11
        return static_cast<size_t>(2 * std::pow(n, 2.0 / 5.0));
    }

    void CSecond_Iteration::Execute_On_CPU(TValues& local_values, const CFile_Reader<double>::TData_Block& data_block, size_t offset)
    {
        double delta;
        double tmp_value;

        for (auto i = offset; i < data_block.count; ++i)
        {
            double value = data_block.data[i];
            if (utils::Is_Valid_Double(value))
            {
                if (m_basic_values->min < 0)
                {
                    value /= config::processing::Scale_Factor;
                }

                delta = value - m_basic_values->mean;
                tmp_value = delta;
                delta /= static_cast<double>(m_basic_values->count - 1);
                delta *= tmp_value;

                local_values.var += delta;
                local_values.histogram->Add(value);
            }
        }
    }

    void CSecond_Iteration::Execute_On_GPU(TValues& local_values, CFile_Reader<double>::TData_Block& data_block, kernels::TOpenCL_Settings& opencl)
    {
        const auto opencl_report = Execute_OpenCL(opencl, data_block, local_values);

        if (!opencl_report.success)
        {
            Execute_On_CPU(local_values, data_block);
        }
        else if (!opencl_report.all_processed)
        {
            const size_t offset = data_block.count - (data_block.count % opencl.work_group_size);
            Execute_On_CPU(local_values, data_block, offset);
        }
    }
}