#include <limits>
#include <future>
#include <vector>
#include <cmath>
#include <iostream>

#include "../utils/utils.h"
#include "../utils/singleton.h"
#include "../utils/resource_manager.h"
#include "../utils/resource_guard.h"
#include "first_iteration.h"

namespace kiv_ppr
{
    CFirst_Iteration::CFirst_Iteration(CFile_Reader<double>* file) noexcept
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
        // Seek to the beginning of the input file.
        m_file->Seek_Beg();

        // Create a new watchdog instance.
        CWatchdog watchdog(thread_config->watchdog_expiration_sec);

        // Create a container for all the workers.
        std::vector<std::future<int>> workers(thread_config->number_of_threads);
        for (auto& worker : workers)
        {
            worker = std::async(std::launch::async, &CFirst_Iteration::Worker, this, thread_config, &watchdog);
        }

        // Execute the workers and add up their return values.
        // If all goes well, all return values should be 0.
        int return_values = 0;
        for (auto& worker : workers)
        {
            return_values += worker.get();
        }

        // Stop the watchdog.
        watchdog.Stop();

        // Aggregate the final mean from all the local means.
        for (const auto& [mean, count] : m_worker_means)
        {
            m_values.mean += mean * (static_cast<double>(count) / static_cast<double>(m_values.count));
        }

        // Check if the entire file has been read and none of the workers returned 1 (error).
        if (return_values != 0 || watchdog.Get_Counter_Value() != m_file->Get_Number_Of_Elements())
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
        m_values.all_ints = m_values.all_ints && values.all_ints;

        // Store the local mean from the worker along with how many 
        // values the worker has processed (need for the final mean aggregation).
        m_worker_means.emplace_back(values.mean, values.count);
    }

    inline CFirst_Iteration::TValues CFirst_Iteration::Aggregate_Results_From_GPU(
                const std::vector<double>& out_min,
                const std::vector<double>& out_max,
                const std::vector<double>& out_mean,
                const std::vector<cl_ulong>& out_count,
                size_t total_count) noexcept
    {
        TValues values{};
        const size_t size = out_min.size(); // All the sizes are the same (work group count).

        for (size_t i = 0; i < size; ++i)
        {
            // Update the minimum.
            if (utils::Is_Valid_Double(out_min.at(i)))
            {
                values.min = std::min(values.min, out_min.at(i));
            }

            // Update the maximum.
            if (utils::Is_Valid_Double(out_max.at(i)))
            {
                values.max = std::max(values.max, out_max.at(i));
            }

            // Update the mean (with regards to how many values each work group has processed).
            if (utils::Is_Valid_Double(out_mean.at(i)))
            {
                values.mean += out_mean.at(i) * (out_count.at(i) / static_cast<double>(total_count));
            }
        }

        return values;
    }

    CFirst_Iteration::TValues CFirst_Iteration::Process_Data_Block_On_CPU(const CFile_Reader<double>::TData_Block& data_block, size_t offset) noexcept
    {
        TValues values{};
        double delta{};

        for (size_t i = offset; i < data_block.count; ++i)
        {
            // The value has to to be a valid double.
            if (utils::Is_Valid_Double(data_block.data[i]))
            {
                // Scale it down, so we are able to calculate -DOUBLE_MAX - DOUBLE_MAX.
                const double value = data_block.data[i] / config::processing::Scale_Factor;

                // Update the minimum and maximum.
                values.min = std::min(values.min, value);
                values.max = std::max(values.max, value);

                // Update the mean.
                ++values.count;
                delta = value - values.mean;
                values.mean += delta / values.count;

                // Check if the value is an integer or not.
                if (values.all_ints && (std::floor(data_block.data[i]) != std::ceil(data_block.data[i])))
                {
                    values.all_ints = false;
                }
            }
        }

        return values;
    }

    CFirst_Iteration::TOpenCL_Report CFirst_Iteration::Execute_OpenCL(kernels::TOpenCL_Settings& opencl, const CFile_Reader<double>::TData_Block& data_block)
    {
        // Calculate how many work groups will be needed.
        const auto work_groups_count = data_block.count / opencl.work_group_size;

        // Calculate how many values we will be able calculate (must be a multiple of the size of one work group).
        const size_t count = data_block.count - (data_block.count % opencl.work_group_size);

        // The number of input values is less than the size of one work group.
        // All work will be done by the CPU.
        if (0 == work_groups_count)
        {
            return { false, false, {} };
        }

        // Create a buffer for the input values. 
        cl::Buffer data_buff(opencl.context, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_USE_HOST_PTR, count * sizeof(double), data_block.data.get());

        // Create output buffers (results calculated by each work group).
        cl::Buffer out_mean_buff(opencl.context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, work_groups_count * sizeof(double));
        cl::Buffer out_min_buff(opencl.context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, work_groups_count * sizeof(double));
        cl::Buffer out_max_buff(opencl.context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, work_groups_count * sizeof(double));
        cl::Buffer out_count_buff(opencl.context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, work_groups_count * sizeof(cl_ulong));
        cl::Buffer out_all_ints_buff(opencl.context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, work_groups_count * sizeof(int));

        // Pass the arguments into the kernel. 
        try
        {
            opencl.kernel.setArg(0 , data_buff);

            opencl.kernel.setArg(1 , opencl.work_group_size * sizeof(double), nullptr);
            opencl.kernel.setArg(2 , opencl.work_group_size * sizeof(double), nullptr);
            opencl.kernel.setArg(3 , opencl.work_group_size * sizeof(double), nullptr);
            opencl.kernel.setArg(4 , opencl.work_group_size * sizeof(int), nullptr);
            opencl.kernel.setArg(5 , opencl.work_group_size * sizeof(cl_ulong), nullptr);

            opencl.kernel.setArg(6 , out_min_buff);
            opencl.kernel.setArg(7 , out_max_buff);
            opencl.kernel.setArg(8 , out_mean_buff);
            opencl.kernel.setArg(9 , out_all_ints_buff);
            opencl.kernel.setArg(10, out_count_buff);
        }
        catch (const cl::Error& e)
        {
            kernels::Print_OpenCL_Error(e, *opencl.device);
            std::exit(7);
        }

        // Create output CPU buffers to store the results from the OpenCL device to.
        std::vector<double> out_min(work_groups_count);
        std::vector<double> out_max(work_groups_count);
        std::vector<double> out_mean(work_groups_count);
        std::vector<cl_ulong> out_count(work_groups_count);
        std::vector<int> out_all_ints(work_groups_count);
        
        try
        {
            // Create a command queue to communicate with the OpenCL device.
            cl::CommandQueue cmd_queue(opencl.context, *opencl.device);

            // Pass the kernel into the OpenCL device ("start the program").
            cmd_queue.enqueueNDRangeKernel(opencl.kernel, cl::NullRange, cl::NDRange(count), cl::NDRange(opencl.work_group_size));

            // Read the results from the OpenCL device.
            cmd_queue.enqueueReadBuffer(out_mean_buff, CL_TRUE, 0, out_mean.size() * sizeof(double), out_mean.data());
            cmd_queue.enqueueReadBuffer(out_min_buff, CL_TRUE, 0, out_min.size() * sizeof(double), out_min.data());
            cmd_queue.enqueueReadBuffer(out_max_buff, CL_TRUE, 0, out_max.size() * sizeof(double), out_max.data());
            cmd_queue.enqueueReadBuffer(out_count_buff, CL_TRUE, 0, out_count.size() * sizeof(cl_ulong), out_count.data());
            cmd_queue.enqueueReadBuffer(out_all_ints_buff, CL_TRUE, 0, out_all_ints.size() * sizeof(int), out_all_ints.data());
        }
        catch (const cl::Error& e)
        {
            kernels::Print_OpenCL_Error(e, *opencl.device);
            std::exit(9);
        }

        size_t number_of_valid_doubles = 0;
        bool all_ints = true;

        // Calculate the total sum of valid doubles as well as if all values are integers.
        for (size_t i = 0; i < work_groups_count; ++i)
        {
            number_of_valid_doubles += out_count.at(i);
            all_ints = all_ints && out_all_ints.at(i);
        }

        // Aggregate the values calculated by individual worker groups.
        const auto aggregated_vals = Aggregate_Results_From_GPU(out_min, out_max, out_mean, out_count, number_of_valid_doubles);

        // Return an OpenCL report (how many numbers were processed, calculated values, etc.)
        return { true, count == data_block.count, {
            aggregated_vals.min,
            aggregated_vals.max,
            aggregated_vals.mean,
            number_of_valid_doubles,
            all_ints
        } };
    }

    int CFirst_Iteration::Worker(const config::TThread_Params* thread_config, CWatchdog* watchdog)
    {
        TValues local_values{}; // Local values (each worker has its own).

        // Make sure that watchdog is not NULL
        if (nullptr == watchdog)
        {
            std::cout << "Error: instance of CWatch_Dog is NULL" << std::endl;
            std::exit(20);
        }

        // Make sure that the resource manager is not NULL.
        auto resource_manager = Singleton<CResource_Manager>::Get_Instance();
        if (nullptr == resource_manager)
        {
            std::cout << "Error: resource manager is NULL" << std::endl;
            std::exit(20);
        }

        const cl::Device* device = nullptr;
        kernels::TOpenCL_Settings opencl;
        CResource_Guard opencl_device_guard;
        bool use_cpu = false;

        // Get the mode in which the program was started (SMP, ALL, ...).
        const auto run_type = resource_manager->Get_Run_Type();

        // If the user entered the 'SMP' mode use the CPU.
        if (run_type == CArg_Parser::NRun_Type::SMP)
        {
            use_cpu = true;
        }
        if (run_type == CArg_Parser::NRun_Type::All || run_type == CArg_Parser::NRun_Type::OpenCL_Devs)
        {
            // Find any OpenCL device available.
            device = resource_manager->Get_Available_Device();

            // If there is no device available and the user ran the program in the 'ALL' mode, use the CPU.
            // Otherwise, return (the user only wants to use OpenCL device).
            if (run_type == CArg_Parser::NRun_Type::OpenCL_Devs && nullptr == device)
            {
                return 0;
            }
            else if (nullptr == device)
            {
                use_cpu = true;
            }
        }
        
        // If the worker obtained an OpenCL device.
        if (nullptr != device)
        {
            opencl = kernels::Init_OpenCL(device, kernels::First_Iteration_Kernel, kernels::First_Iteration_Kernel_Name);
            kernels::Adjust_Work_Group_Size(opencl, kernels::First_Iteration_Get_Size_Of_Local_Params);
            opencl_device_guard.Set_Device(device);
        }

        // Start the watchdog
        watchdog->Start();

        while (true)
        {
            // Read a block of data.
            auto data_block = m_file->Read_Data(thread_config->number_of_elements_per_file_read);

            switch (data_block.status)
            {
                // Process the block of data (either on a CPU or an OpenCL device).
                case kiv_ppr::CFile_Reader<double>::NRead_Status::OK:
                    if (use_cpu)
                    {
                        Execute_On_CPU(local_values, data_block);
                    }
                    else
                    {
                        Execute_On_GPU(local_values, data_block, opencl);
                    }

                    // Kick the watchdog.
                    watchdog->Kick(data_block.count);
                    break;

                // The end of the file has been reached, so report
                // the results (local values to the farmer).
                case CFile_Reader<double>::NRead_Status::EOF_:
                    Report_Worker_Results(local_values);
                    return 0;

                // An error has ocurred. Inform the farmer that we failed to read the file.
                case CFile_Reader<double>::NRead_Status::Error: [[fallthrough]];
                default:
                    return 1;
            }
        }
    }

    void CFirst_Iteration::Execute_On_CPU(TValues& local_values, const CFile_Reader<double>::TData_Block& data_block) noexcept
    {
        double delta{};

        for (auto i = 0; i < data_block.count; ++i)
        {
            double value = data_block.data[i];

            // Filter out valid doubles.
            if (utils::Is_Valid_Double(value))
            {
                // Update all_ints.
                if (local_values.all_ints && (std::floor(value) != std::ceil(value)))
                {
                    local_values.all_ints = false;
                }

                // Scale the value down, so the mean does not overflow.
                value /= config::processing::Scale_Factor;

                // Update the minimum and maximum.
                local_values.min = std::min(local_values.min, value);
                local_values.max = std::max(local_values.max, value);

                // Update the mean.
                ++local_values.count;
                delta = value - local_values.mean;
                local_values.mean += delta / static_cast<double>(local_values.count);
            }
        }
    }

    void CFirst_Iteration::Execute_On_GPU(TValues& local_values, const CFile_Reader<double>::TData_Block& data_block, kernels::TOpenCL_Settings& opencl)
    {
        // Process as much data of the block of data on the OpenCL device as you can.
        const auto opencl_report = Execute_OpenCL(opencl, data_block);

        // Store the calculated statistical values.
        TValues gpu_values = opencl_report.values;

        // If the number of values < work_group_size, we have to process it all on the CPU.
        if (!opencl_report.success)
        {
            gpu_values = Process_Data_Block_On_CPU(data_block, 0);
        }
        else if (!opencl_report.all_processed)
        {
            // Process the remaining part on the CPU.
            const size_t offset = data_block.count - (data_block.count % opencl.work_group_size);
            const auto cpu_values = Process_Data_Block_On_CPU(data_block, offset);

            // Merge the values calculated on the CPU into the values calculated on the OpenCL device.
            Merge_Values(gpu_values, cpu_values);
        }

        // Merge the values into local values.
        Merge_Values(local_values, gpu_values);
    }

    void CFirst_Iteration::Merge_Values(TValues& dest, const TValues& src) noexcept
    {
        // Add up the two sub-counts to the total count.
        const size_t total_count = dest.count + src.count;

        // Update the minimum and maximum.
        dest.min = std::min(dest.min, src.min);
        dest.max = std::max(dest.max, src.max);

        // Update the mean (with regards to how many values were used to calculate each sub-mean).
        dest.mean *= (static_cast<double>(dest.count) / static_cast<double>(total_count));
        dest.mean += src.mean * (static_cast<double>(src.count) / static_cast<double>(total_count));

        // Update all_ints.
        dest.all_ints = dest.all_ints && src.all_ints;

        // Update the total count.
        dest.count = total_count;
    }
}

// EOF