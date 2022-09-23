#include <vector>
#include <future>
#include <cmath>

#include "AdvancedFileStats.h"

namespace kiv_ppr
{
    template<class T, class E>
    CAdvanced_File_Stats<T, E>::CAdvanced_File_Stats(CFile_Reader<E> *file,
                                                     std::function<bool(E)> num_valid_fce,
                                                     typename CBasic_File_Stats<T, E>::TValues basic_values,
                                                     typename CHistogram<T>::TConfig histogram_config)
        : m_file(file),
          m_num_valid_fce(num_valid_fce),
          m_basic_values(basic_values),
          m_histogram_config(histogram_config),
          m_standard_deviation{},
          m_histogram{std::make_shared<CHistogram<T>>(histogram_config)}
    {

    }

    template<class T, class E>
    [[nodiscard]] T CAdvanced_File_Stats<T, E>::Get_Standard_Deviation() const noexcept
    {
        return m_standard_deviation;
    }

    template<class T, class E>
    [[nodiscard]] std::shared_ptr<CHistogram<T>> CAdvanced_File_Stats<T, E>::Get_Histogram() const noexcept
    {
        return m_histogram;
    }

    template<class T, class E>
    [[nodiscard]] typename CAdvanced_File_Stats<T, E>::TValues CAdvanced_File_Stats<T, E>::Get_Values() const noexcept
    {
        return
        {
            Get_Standard_Deviation(),
            Get_Histogram()
        };
    }

    template<class T, class E>
    [[nodiscard]] int CAdvanced_File_Stats<T, E>::Process(config::TThread_Config thread_config)
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

    template<class T, class E>
    void CAdvanced_File_Stats<T, E>::Report_Results(T standard_deviation, CHistogram<T> &histogram) noexcept
    {
        const std::lock_guard<std::mutex> lock(m_mtx);

        m_standard_deviation += standard_deviation;
        m_histogram->operator+=(histogram);
    }

    template<class T, class E>
    [[nodiscard]] int CAdvanced_File_Stats<T, E>::Worker(const config::TThread_Config* thread_config, CWatch_Dog* watch_dog) noexcept
    {
        CHistogram<T> histogram(m_histogram_config);
        T standard_deviation{};
        T tmp_value1;
        T tmp_value2;

        watch_dog->Register();
        while (true)
        {
            watch_dog->Kick();

            const auto [status, count, data] = m_file->Read_Data(thread_config->number_of_elements_per_file_read);
            switch (status)
            {
                case kiv_ppr::CFile_Reader<E>::NStatus::OK:
                    for (size_t i = 0; i < count; ++i)
                    {
                        if (m_num_valid_fce(data[i]))
                        {
                            tmp_value1 = static_cast<T>(data[i]) - m_basic_values.mean;
                            tmp_value2 = tmp_value1;
                            tmp_value1 /= (m_file->Get_Total_Number_Of_Valid_Elements() - 1);
                            tmp_value1 *= tmp_value2;
                            standard_deviation += tmp_value1;

                            histogram.Add(data[i]);
                        }
                    }
                    break;
                case CFile_Reader<E>::NStatus::ERROR:
                    watch_dog->Unregister();
                    return 1;
                case CFile_Reader<E>::NStatus::EOF_:
                    watch_dog->Unregister();
                    Report_Results(standard_deviation, histogram);
                    return 0;
            }
        }
    }

    template class CAdvanced_File_Stats<double, double>;
}