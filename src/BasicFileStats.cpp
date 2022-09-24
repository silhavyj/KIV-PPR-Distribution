#include <limits>
#include <iostream>
#include <utility>

#include "BasicFileStats.h"

namespace kiv_ppr
{
    CBasic_File_Stats::CBasic_File_Stats(CFile_Reader<double>* file,
                                         std::function<bool(double)> num_valid_fce)
        : m_file(file),
          m_num_valid_fce(std::move(num_valid_fce)),
          m_min{std::numeric_limits<double>::max()},
          m_max{std::numeric_limits<double>::min()},
          m_mean{}
    {

    }

    [[nodiscard]] double CBasic_File_Stats::Get_Min() const noexcept
    {
        return m_min;
    }

    [[nodiscard]] double CBasic_File_Stats::Get_Max() const noexcept
    {
        return m_max;
    }

    [[nodiscard]] double CBasic_File_Stats::Get_Mean() const noexcept
    {
        return m_mean;
    }

    [[nodiscard]] typename CBasic_File_Stats::TValues CBasic_File_Stats::Get_Values() const noexcept
    {
        return
        {
            Get_Min(),
            Get_Max(),
            Get_Mean()
        };
    }

    [[nodiscard]] int CBasic_File_Stats::Process(config::TThread_Config thread_config)
    {
        m_file->Seek_Beg();
        CWatch_Dog watch_dog(thread_config.watchdog_expiration_sec);

        std::vector<std::future<int>> workers(thread_config.number_of_threads);
        for (auto& worker : workers)
        {
            worker = std::async(std::launch::async, &CBasic_File_Stats::Worker, this, &thread_config, &watch_dog);
        }

        int ret_values = 0;
        for (auto& worker : workers)
        {
            ret_values += worker.get();
        }
        if (watch_dog.Get_Number_Of_Registered_Threads() != 0 || ret_values != 0)
        {
            return 1;
        }
        return 0;
    }

    void CBasic_File_Stats::Report_Results(double min, double max, double mean) noexcept
    {
        const std::lock_guard<std::mutex> lock(m_mtx);

        m_min = std::min(m_min, min);
        m_max = std::max(m_max, max);
        m_mean += mean;
    }

    [[nodiscard]] int CBasic_File_Stats::Worker(const config::TThread_Config* thread_config, CWatch_Dog* watch_dog)
    {
        double min = std::numeric_limits<double>::max();
        double max = std::numeric_limits<double>::min();
        double mean{};

        watch_dog->Register();
        while (true)
        {
            watch_dog->Kick();

            const auto [status, count, data] = m_file->Read_Data(thread_config->number_of_elements_per_file_read);
            switch (status)
            {
                case kiv_ppr::CFile_Reader<double>::NStatus::OK:
                    for (long i = 0; i < count; ++i)
                    {
                        const double value = data[i];
                        if (m_num_valid_fce(value))
                        {
                            min = std::min(min, value);
                            max = std::max(max, value);
                            mean += value / static_cast<double>(m_file->Get_Total_Number_Of_Valid_Elements());
                        }
                    }
                    break;
                case CFile_Reader<double>::NStatus::ERROR:
                    watch_dog->Unregister();
                    return 1;
                case CFile_Reader<double>::NStatus::EOF_:
                    watch_dog->Unregister();
                    Report_Results(min, max, mean);
                    return 0;
            }
        }
    }
}