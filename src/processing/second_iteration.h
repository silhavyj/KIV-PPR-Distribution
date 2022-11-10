#pragma once

#include <memory>
#include <functional>

#include "first_iteration.h"
#include "histogram.h"
#include "../utils/file_reader.h"
#include "../utils/watchdog.h"

namespace kiv_ppr
{
    class CSecond_Iteration
    {
    public:
        struct TValues
        {
            double var = 0.0;
            double sd = 0.0;
            std::shared_ptr<CHistogram> histogram = nullptr;
        };

    public:
        explicit CSecond_Iteration(CFile_Reader<double>* file,
                                   typename CFirst_Iteration::TValues* basic_values);

        ~CSecond_Iteration() = default;

        [[nodiscard]] TValues Get_Values() const noexcept;
        [[nodiscard]] int Run(config::TThread_Params* thread_config);

    private:
        struct TOpenCL_Report
        {
            bool success;
            bool all_processed;
        };

    private:
        void Report_Worker_Results(const TValues& values);
        [[nodiscard]] int Worker(const config::TThread_Params* thread_config, CWatchdog* watchdog);
        static size_t Calculate_Number_Of_Intervals(size_t n) noexcept;
        void Scale_Up_Basic_Values(typename CFirst_Iteration::TValues* basic_values) noexcept;
        void Execute_On_CPU(TValues& local_values, const CFile_Reader<double>::TData_Block& data_block, size_t offset = 0) noexcept;
        void Execute_On_GPU(TValues& local_values, const CFile_Reader<double>::TData_Block& data_block, kernels::TOpenCL_Settings& opencl);
        TOpenCL_Report Execute_OpenCL(kernels::TOpenCL_Settings& opencl, const CFile_Reader<double>::TData_Block& data_block, TValues& local_values);

    private:
        CFile_Reader<double>* m_file;
        typename CFirst_Iteration::TValues* m_basic_values;
        TValues m_values;
        std::mutex m_mtx;
        CHistogram::TParams m_histogram_params;
    };
}

// EOF