#pragma once

#include <memory>
#include <functional>

#include "FirstIteration.h"
#include "../Histogram.h"
#include "../FileReader.h"

namespace kiv_ppr
{
    class CSecond_Iteration
    {
    public:
        struct TValues
        {
            double var;
            double sd;
            std::shared_ptr<CHistogram> histogram;

            friend std::ostream& operator<<(std::ostream& out, const TValues& values);
        };

    public:
        explicit CSecond_Iteration(CFile_Reader<double>* file,
                                   std::function<bool(double)> is_valid_number,
                                   typename CFirst_Iteration::TValues basic_values);

        ~CSecond_Iteration() = default;

        [[nodiscard]] double Get_Var() const noexcept;
        [[nodiscard]] double Get_SD() const noexcept;
        [[nodiscard]] std::shared_ptr<CHistogram> Get_Histogram() const noexcept;
        [[nodiscard]] TValues Get_Values() const noexcept;

        [[nodiscard]] int Run(config::TThread_Params* thread_config);

    private:
        void Report_Worker_Results(const TValues& values);
        [[nodiscard]] int Worker(config::TThread_Params* thread_config);
        static size_t Calculate_Number_Of_Intervals(size_t n);

    private:
        CFile_Reader<double>* m_file;
        std::function<bool(double)> m_is_valid_number;
        typename CFirst_Iteration::TValues m_basic_values;
        TValues m_values;
        std::mutex m_mtx;
        CHistogram::TParams m_histogram_params;
    };
}