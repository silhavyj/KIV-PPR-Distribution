#pragma once

#include <mutex>
#include <utility>
#include <functional>

#include "../config.h"
#include "../utils/file_reader.h"
#include "../utils/watchdog.h"

namespace kiv_ppr
{
    class CFirst_Iteration
    {
    public:
        using Worker_Mean_t = std::pair<double, size_t>;

        struct TValues
        {
            double min;
            double max;
            double mean;
            size_t count;
            bool all_ints;
        };

    public:
        explicit CFirst_Iteration(CFile_Reader<double>* file,
                                  std::function<bool(double)> is_valid_number);

        ~CFirst_Iteration() = default;

        [[nodiscard]] TValues Get_Values() const noexcept;
        [[nodiscard]] int Run(config::TThread_Params* thread_config);

    private:
        void Report_Worker_Results(TValues values);
        [[nodiscard]] int Worker(config::TThread_Params* thread_config, CWatchdog* watchdog);

    private:
        CFile_Reader<double>* m_file;
        std::function<bool(double)> m_is_valid_number;
        TValues m_values;
        std::mutex m_mtx;
        std::vector<Worker_Mean_t> m_worker_means;
    };
}