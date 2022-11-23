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
        // Scale up the values calculated in the first iteration.
        if (m_basic_values->min >= 0)
        {
            Scale_Up_Basic_Values(m_basic_values);
        }

        // Create parameters of the histogram.
        m_histogram_params = {
            m_basic_values->min,
            m_basic_values->max,
            Calculate_Number_Of_Intervals(m_basic_values->count)
        };
        
        // Create the histogram itself.
        m_values.histogram = std::make_shared<CHistogram>(m_histogram_params);
    }

    void CSecond_Iteration::Scale_Up_Basic_Values(typename CFirst_Iteration::TValues* basic_values) noexcept
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
        // Seek to the beginning of the input file.
        m_file->Seek_Beg();

        // Create a new watchdog instance.
        CWatchdog watchdog(thread_config->watchdog_expiration_sec);

        // Create a container for all the workers.
        std::vector<std::future<int>> workers(thread_config->number_of_threads);
        for (auto& worker : workers)
        {
            worker = std::async(std::launch::async, &CSecond_Iteration::Worker, this, thread_config, &watchdog);
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

        // Calculate the standard deviation.
        m_values.sd = std::sqrt(m_values.var);

        // Check if the entire file has been read and none of the workers returned 1 (error).
        if (return_values != 0 || watchdog.Get_Counter_Value() != m_file->Get_Number_Of_Elements())
        {
            return 1;
        }

        return 0;
    }

    void CSecond_Iteration::Report_Worker_Results(const TValues& values)
    {
        const std::lock_guard<std::mutex> lock(m_mtx);
        
        // Update the variance.
        m_values.var += values.var;

        // Merge the local histogram with the global one.
        m_values.histogram->operator+=(*values.histogram);
    }

    CSecond_Iteration::TOpenCL_Report CSecond_Iteration::Execute_OpenCL(kernels::TOpenCL_Settings& opencl, const CFile_Reader<double>::TData_Block& data_block, TValues& local_values)
    {
        // Calculate how many work groups will be needed.
        const auto work_groups_count = data_block.count / opencl.work_group_size;

        // Calculate how many values we will be able calculate (must be a multiple of the size of one work group).
        const size_t count = data_block.count - (data_block.count % opencl.work_group_size);

        // The number of input values is less than the size of one work group.
        // All work will be done by the CPU.
        if (0 == work_groups_count)
        {
            return { false, false };
        }

        // Retrieve the interval size (bin width) as well as the number of intervals. 
        const size_t number_of_intervals = local_values.histogram->Get_Number_Of_Intervals();
        const double interval_size = local_values.histogram->Get_Interval_Size();

        // Create a vector (histogram) that will be passed to the OpenCL device.
        // The histogram is twice the size of the original one - not every OpenCL device
        // can perform an atomic operation on size_t (bin), therefore each bin is represented as 
        // two uint values (carry bit).
        std::vector<cl_uint> out_histogram(2 * number_of_intervals, 0);

        // Create output buffers (results calculated by each work group).
        cl::Buffer data_buff(opencl.context, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_USE_HOST_PTR, count * sizeof(double), data_block.data.get());
        cl::Buffer out_var_buff(opencl.context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, work_groups_count * sizeof(double));
        cl::Buffer histogram_buff(opencl.context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, out_histogram.size() * sizeof(cl_uint), out_histogram.data());

        // Pass the arguments into the kernel. 
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

        // Create output CPU buffers to store the results from the OpenCL device to.
        std::vector<double> out_var(work_groups_count);
   
        try
        {
            // Create a command queue to communicate with the OpenCL device.
            cl::CommandQueue cmd_queue(opencl.context, *opencl.device);

            // Pass the kernel into the OpenCL device ("start the program").
            cmd_queue.enqueueNDRangeKernel(opencl.kernel, cl::NullRange, cl::NDRange(count), cl::NDRange(opencl.work_group_size));

            // Read the results from the OpenCL device.
            cmd_queue.enqueueReadBuffer(out_var_buff, CL_TRUE, 0, out_var.size() * sizeof(double), out_var.data());
            cmd_queue.enqueueReadBuffer(histogram_buff, CL_TRUE, 0, out_histogram.size() * sizeof(cl_uint), out_histogram.data());
        }
        catch (const cl::Error& e)
        {
            kernels::Print_OpenCL_Error(e, *opencl.device);
            std::exit(13);
        }

        // Update the local histogram (add up the values calculated on the OpenCL device)
        for (size_t i = 0; i < number_of_intervals; ++i)
        {
            // Each interval is represented as two uint variable. 
            const size_t value = static_cast<size_t>(out_histogram.at(2 * i)) + static_cast<size_t>(out_histogram.at(2 * i + 1)) * sizeof(cl_uint);

            if (false == local_values.histogram->Add(i, value))
            {
                std::cout << "Failed to update the histogram" << std::endl;
                std::exit(14);
            }
        }

        // Add up the variance values from the individual work groups.
        double gpu_var = 0.0;
        for (const auto& value : out_var)
        {
            gpu_var += value;
        }

        // Update the local variance.
        local_values.var += gpu_var;

        return { true, count == data_block.count };
    }

    int CSecond_Iteration::Worker(const config::TThread_Params* thread_config, CWatchdog* watchdog)
    {
        // Local values (each worker has its own).
        TValues local_values{}; 
        local_values.histogram = std::make_shared<CHistogram>(m_histogram_params);

        // Make sure that watchdog is not NULL
        if (nullptr == watchdog)
        {
            std::cout << "Error: instance of CWatch_Dog is NULL" << std::endl;
            std::exit(22);
        }

        // Make sure that the resrouce manager is not NULL.
        auto resource_manager = Singleton<CResource_Manager>::Get_Instance();
        if (nullptr == resource_manager)
        {
            std::cout << "Error: resource manager is NULL" << std::endl;
            std::exit(23);
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
            opencl = kernels::Init_OpenCL(device, kernels::Second_Iteration_Kernel, kernels::Second_Iteration_Kernel_Name);
            kernels::Adjust_Work_Group_Size(opencl, kernels::Second_Iteration_Get_Size_Of_Local_Params);
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

    size_t CSecond_Iteration::Calculate_Number_Of_Intervals(size_t n) noexcept
    {
        return static_cast<size_t>(2 * std::pow(n, 2.0 / 5.0));
    }

    void CSecond_Iteration::Execute_On_CPU(TValues& local_values, const CFile_Reader<double>::TData_Block& data_block, size_t offset)
    {
        const __m256d _count_minus_1 = _mm256_set1_pd(static_cast<double>(m_basic_values->count) - 1);
        const __m256d _mean = _mm256_set1_pd(m_basic_values->mean);
        __m256d _var = _mm256_set1_pd(0);

        std::array<double, 4> valid_doubles{};
        std::size_t index = 0;

        for (auto i = offset; i < data_block.count; ++i)
        {
            double value = data_block.data[i];

            // The value has to to be a valid double.
            if (utils::Is_Valid_Double(value))
            {
                // Scale the value down if necessary.
                if (m_basic_values->min < 0)
                {
                    value /= config::processing::Scale_Factor;
                }

                // Add the value into the array of valid doubles.
                valid_doubles.at(index) = value;

                // Once we have 4 valid doubles, udpate the variance
                // using SIMD instructions of the CPU.
                if (index == 3)
                {
                    index = 0;
                    Update_Variance(valid_doubles, _var, _mean, _count_minus_1);
                }
                else
                {
                    ++index;
                }

                // Update the local histogram.
                local_values.histogram->Add(value);
            }
        }
        // There might be some values left.
        if (index != 0)
        {
            // The "unused" spots are set to m_basic_values->mean, which
            // does not have any impact on the final result as the difference will be 0.
            for (std::size_t i = index; i < 4; ++i)
            {
                valid_doubles.at(i) = m_basic_values->mean;
            }
            Update_Variance(valid_doubles, _var, _mean, _count_minus_1);
        }

        // Aggeregate (sum up) all the values.
        local_values.var = utils::vectorization::Aggregate(_var, 0.0, [](double x, double y) { return x + y; });
    }

    void CSecond_Iteration::Execute_On_GPU(TValues& local_values, const CFile_Reader<double>::TData_Block& data_block, kernels::TOpenCL_Settings& opencl)
    {
        // Process as much data of the block of data on the OpenCL device as you can.
        const auto opencl_report = Execute_OpenCL(opencl, data_block, local_values);

        // If the number of values < work_group_size, we have to process it all on the CPU.
        if (!opencl_report.success)
        {
            Execute_On_CPU(local_values, data_block);
        }
        else if (!opencl_report.all_processed)
        {
            // Process the remaining part on the CPU.
            const size_t offset = data_block.count - (data_block.count % opencl.work_group_size);
            Execute_On_CPU(local_values, data_block, offset);
        }
    }

    void CSecond_Iteration::Update_Variance(const std::array<double, 4>& valid_doubles, __m256d& _var, const __m256d& _mean, const __m256d& _count_minus_1) noexcept
    {
        // Add the four doubles into __m256d.
        const __m256d _vals = _mm256_set_pd(
            valid_doubles.at(0),
            valid_doubles.at(1),
            valid_doubles.at(2),
            valid_doubles.at(3)
        );

        __m256d _delta = _mm256_sub_pd(_vals, _mean);
        const __m256d _tmp_value = _delta;
        _delta = _mm256_div_pd(_delta, _count_minus_1);
        _delta = _mm256_mul_pd(_delta, _tmp_value);
        _var = _mm256_add_pd(_var, _delta);
    }
}

// EOF