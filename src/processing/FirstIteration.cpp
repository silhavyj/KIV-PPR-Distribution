#include <limits>
#include <future>
#include <vector>
#include <cmath>
#include <iostream>

#include "FirstIteration.h"

namespace kiv_ppr
{
    CFirst_Iteration::CFirst_Iteration(CFile_Reader<double>* file,
                                       std::function<bool(double)> is_valid_number)
        : m_file(file),
          m_is_valid_number(std::move(is_valid_number)),
          m_values{},
          m_worker_means{}
    {
        m_values.all_ints = true;
        m_values.min = std::numeric_limits<double>::max();
        m_values.max = std::numeric_limits<double>::lowest();
    }

    typename CFirst_Iteration::TValues CFirst_Iteration::Get_Values() const noexcept
    {
        return m_values;
    }

    int CFirst_Iteration::Run(config::TThread_Params* thread_config)
    {
        m_file->Seek_Beg();

        std::vector<std::future<int>> workers(thread_config->number_of_threads);
        CWatchdog watchdog(thread_config->watchdog_expiration_sec);
        for (auto& worker : workers)
        {
            worker = std::async(std::launch::async, &CFirst_Iteration::Worker, this, thread_config, &watchdog);
        }

        int return_values = 0;
        for (auto& worker : workers)
        {
            return_values += worker.get();
        }
        watchdog.Stop();

        for (const auto& [mean, count] : m_worker_means)
        {
            m_values.mean += mean * (static_cast<double>(count) / static_cast<double>(m_values.count));
        }

        if (return_values != 0 || watchdog.Get_Counter_Value() != (m_file->Get_File_Size() / sizeof(double)))
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

    int CFirst_Iteration::Worker(config::TThread_Params* thread_config, CWatchdog* watchdog)
    {
        TValues local_values {
            std::numeric_limits<double>::max(),
            std::numeric_limits<double>::lowest(),
            0.0,
            0,
            true
        };
        double delta;

        while (true)
        {
            const auto [status, count, data] = m_file->Read_Data(thread_config->number_of_elements_per_file_read);
            switch (status)
            {
                case kiv_ppr::CFile_Reader<double>::NRead_Status::OK:
                    for (auto i = 0; i < count; ++i)
                    {
                        double value = data[i];
                        if (m_is_valid_number(value))
                        {
                            if (local_values.all_ints && (std::floor(value) != std::ceil(value)))
                            {
                                local_values.all_ints = false;
                            }

                            value /= config::SCALE_FACTOR;

                            local_values.min = std::min(local_values.min, value);
                            local_values.max = std::max(local_values.max, value);

                            ++local_values.count;
                            delta = value - local_values.mean;
                            local_values.mean += delta / static_cast<double>(local_values.count);
                        }
                    }
                    watchdog->Kick(count);
                    break;

                case CFile_Reader<double>::NRead_Status::EOF_:
                    Report_Worker_Results(local_values);
                    return 0;

                case CFile_Reader<double>::NRead_Status::ERROR: [[fallthrough]];
                default:
                    return 1;
            }
        }
    }
}