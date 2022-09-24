#include <utility>
#include <vector>
#include <future>
#include <cmath>

#include "AdvancedFileStats.h"

namespace kiv_ppr
{
    CAdvanced_File_Stats::CAdvanced_File_Stats(CFile_Reader<double> *file,
                                               std::function<bool(double)> num_valid_fce,
                                               typename CBasic_File_Stats::TValues basic_values,
                                               typename CHistogram::TConfig histogram_config)
        : m_file(file),
          m_num_valid_fce(std::move(num_valid_fce)),
          m_basic_values(basic_values),
          m_histogram_config(histogram_config),
          m_standard_deviation{},
          m_histogram{std::make_shared<CHistogram>(histogram_config)}
    {

    }

    [[nodiscard]] double CAdvanced_File_Stats::Get_Standard_Deviation() const noexcept
    {
        return m_standard_deviation;
    }

    [[nodiscard]] std::shared_ptr<CHistogram> CAdvanced_File_Stats::Get_Histogram() const noexcept
    {
        return m_histogram;
    }

    [[nodiscard]] typename CAdvanced_File_Stats::TValues CAdvanced_File_Stats::Get_Values() const noexcept
    {
        return
        {
            Get_Standard_Deviation(),
            Get_Histogram()
        };
    }

    [[nodiscard]] int CAdvanced_File_Stats::Process(config::TThread_Config thread_config)
    {
        m_file->Seek_Beg();
        CWatch_Dog watch_dog(thread_config.watchdog_expiration_sec);

        std::vector<std::future<int>> workers(thread_config.number_of_threads);
        for (auto& worker : workers)
        {
            worker = std::async(std::launch::async, &CAdvanced_File_Stats::Worker, this, &thread_config, &watch_dog);
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
        m_standard_deviation = std::sqrt(m_standard_deviation);
        return 0;
    }

    void CAdvanced_File_Stats::Report_Results(double standard_deviation, CHistogram& histogram) noexcept
    {
        const std::lock_guard<std::mutex> lock(m_mtx);

        m_standard_deviation += standard_deviation;
        m_histogram->operator+=(histogram);
    }

    [[nodiscard]] int CAdvanced_File_Stats::Worker(const config::TThread_Config* thread_config, CWatch_Dog* watch_dog) noexcept
    {
        CHistogram histogram(m_histogram_config);
        double standard_deviation{};
        double tmp_value1;
        double tmp_value2;

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
                            tmp_value1 = value - m_basic_values.mean;
                            tmp_value2 = tmp_value1;
                            tmp_value1 /= static_cast<double>(m_file->Get_Total_Number_Of_Valid_Elements() - 1);
                            tmp_value1 *= tmp_value2;
                            standard_deviation += tmp_value1;

                            histogram.Add(value);
                        }
                    }
                    break;
                case CFile_Reader<double>::NStatus::ERROR:
                    watch_dog->Unregister();
                    return 1;
                case CFile_Reader<double>::NStatus::EOF_:
                    watch_dog->Unregister();
                    Report_Results(standard_deviation, histogram);
                    return 0;
            }
        }
    }
}