#include <limits>
#include <vector>
#include <future>
#include <execution>

#include "BasicFileStats.h"

namespace kiv_ppr
{
    template<class T, class E>
    CBasic_File_Stats<T, E>::CBasic_File_Stats(CFile_Reader<E>* file, std::function<bool(E)> num_valid_fce)
        : m_file(file),
          m_min{std::numeric_limits<T>::max()},
          m_max{std::numeric_limits<T>::min()},
          m_num_valid_fce(num_valid_fce),
          m_mean{}
    {

    }

    template<class T, class E>
    [[nodiscard]] T CBasic_File_Stats<T, E>::Get_Min() const noexcept
    {
        return m_min;
    }

    template<class T, class E>
    [[nodiscard]] T CBasic_File_Stats<T, E>::Get_Max() const noexcept
    {
        return m_max;
    }

    template<class T, class E>
    [[nodiscard]] T CBasic_File_Stats<T, E>::Get_Mean() const noexcept
    {
        return m_mean;
    }

    template<class T, class E>
    [[nodiscard]] typename CBasic_File_Stats<T, E>::TValues CBasic_File_Stats<T, E>::Get_Values() const noexcept
    {
        return
        {
            Get_Min(),
            Get_Max(),
            Get_Mean()
        };
    }

    template<class T, class E>
    [[nodiscard]] int CBasic_File_Stats<T, E>::Process(config::TThread_Config thread_config)
    {
        m_file->Seek_Beg();
        CWatch_Dog watch_dog(thread_config.kick_to_watch_dog_ms, thread_config.number_of_threads);

        std::vector<std::future<int>> workers(thread_config.number_of_threads);
        for (uint32_t i = 0; i < thread_config.number_of_threads; ++i)
        {
            workers[i] = std::async(std::launch::async, &CBasic_File_Stats::Worker, this, thread_config, &watch_dog);
        }
        std::atomic<int> success = 0;
        std::for_each(std::execution::par, workers.begin(), workers.end(), [&success](auto& worker) {
            success += worker.get();
        });
        return success;
    }

    template<class T, class E>
    void CBasic_File_Stats<T, E>::Report_Results(T min, T max, T mean) noexcept
    {
        const std::lock_guard<std::mutex> lock(m_mtx);

        m_min = std::min(m_min, min);
        m_max = std::max(m_max, max);
        m_mean += mean;
    }

    template<class T, class E>
    [[nodiscard]] int CBasic_File_Stats<T, E>::Worker(const config::TThread_Config& thread_config, CWatch_Dog* watch_dog)
    {
        T min = std::numeric_limits<T>::max();
        T max = std::numeric_limits<T>::min();
        T mean{};

        while (true)
        {
            const auto [status, count, data] = m_file->Read_Data(thread_config.number_of_elements_per_file_read);
            watch_dog->Kick();

            switch (status)
            {
                case kiv_ppr::CFile_Reader<E>::NStatus::OK:
                    for (std::size_t i = 0; i < count; ++i)
                    {
                        if (m_num_valid_fce(data[i]))
                        {
                            min = std::min(min, static_cast<T>(data[i]));
                            max = std::max(max, static_cast<T>(data[i]));
                            mean += static_cast<T>(data[i]) / m_file->Get_Total_Number_Of_Valid_Elements();
                        }
                    }
                    break;
                case CFile_Reader<E>::NStatus::ERROR:
                    watch_dog->Remove();
                    return 1;
                case CFile_Reader<E>::NStatus::EOF_:
                    watch_dog->Remove();
                    Report_Results(min, max, mean);
                    return 0;
            }
        }
    }

    template class CBasic_File_Stats<double, double>;
}