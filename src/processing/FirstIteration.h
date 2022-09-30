#pragma once

#include <mutex>
#include <functional>

#include "../Config.h"
#include "../FileReader.h"

namespace kiv_ppr
{
    class CFirst_Iteration
    {
    public:
        struct TValues
        {
            double min;
            double max;
            double mean;
            size_t count;
            bool all_normal_numbers;

            friend std::ostream& operator<<(std::ostream& out, const TValues& values);
        };

    public:
        explicit CFirst_Iteration(CFile_Reader<double>* file,
                                  std::function<bool(double)> is_valid_number);

        ~CFirst_Iteration() = default;

        [[nodiscard]] double Get_Min() const noexcept;
        [[nodiscard]] double Get_Max() const noexcept;
        [[nodiscard]] double Get_Mean() const noexcept;
        [[nodiscard]] size_t Get_Count() const noexcept;
        [[nodiscard]] bool Get_Is_All_Normal_Numbers() const noexcept;
        [[nodiscard]] TValues Get_Values() const noexcept;

        [[nodiscard]] int Run(config::TThread_Params* thread_config);

    private:
        void Report_Worker_Results(TValues values);
        [[nodiscard]] int Worker(config::TThread_Params* thread_config);

    private:
        CFile_Reader<double>* m_file;
        std::function<bool(double)> m_is_valid_number;
        TValues m_values;
        std::mutex m_mtx;
    };
}