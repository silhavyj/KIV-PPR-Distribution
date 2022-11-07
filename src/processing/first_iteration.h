#pragma once

#include <mutex>
#include <utility>
#include <functional>

#include "../config.h"
#include "../utils/file_reader.h"
#include "../utils/watchdog.h"
#include "gpu_kernels.h"

namespace kiv_ppr
{
    class CFirst_Iteration
    {
    public:
        using Worker_Mean_t = std::pair<double, size_t>;

        struct TValues
        {
            double min = std::numeric_limits<double>::max();
            double max = std::numeric_limits<double>::lowest();
            double mean = 0.0;
            size_t count = 0;
            bool all_ints = true;
        };

    public:
        explicit CFirst_Iteration(CFile_Reader<double>* file);

        ~CFirst_Iteration() = default;

        [[nodiscard]] TValues Get_Values() const noexcept;
        [[nodiscard]] int Run(config::TThread_Params* thread_config);

    private:
        struct TOpenCL_Report
        {
            bool success;
            bool all_processed;
            TValues values;
        };

    private:
        void Report_Worker_Results(TValues values);
        [[nodiscard]] int Worker(config::TThread_Params* thread_config, CWatchdog* watchdog);
        TOpenCL_Report Execute_OpenCL(kernels::TOpenCL_Settings& opencl, CFile_Reader<double>::TData_Block& data_block);
        static TValues Agregate_Results_From_GPU(const std::vector<double>& out_min,
                                                 const std::vector<double>& out_max, 
                                                 const std::vector<double>& out_mean,
                                                 const std::vector<cl_ulong>& out_count,
                                                 size_t total_count);
        void Execute_On_CPU(TValues& local_values, const CFile_Reader<double>::TData_Block& data_block);
        void Execute_On_GPU(TValues& local_values, CFile_Reader<double>::TData_Block& data_block, kernels::TOpenCL_Settings& opencl);
        TValues Process_Data_Block_On_CPU(CFile_Reader<double>::TData_Block& data_block, size_t offset);
        void Merge_Values(TValues& dest, const TValues& src);

    private:
        CFile_Reader<double>* m_file;
        TValues m_values;
        std::mutex m_mtx;
        std::vector<Worker_Mean_t> m_worker_means;
    };
}