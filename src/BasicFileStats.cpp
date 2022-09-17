#include <limits>
#include <vector>
#include <future>

#include "BasicFileStats.h"

namespace kiv_ppr
{
    template<class T, class E>
    Basic_File_Stats<T, E>::Basic_File_Stats(File_Reader<E>* file, uint32_t number_of_threads)
        : m_file(file),
          m_number_of_threads(number_of_threads),
          m_min{std::numeric_limits<T>::max()},
          m_max{std::numeric_limits<T>::min()},
          m_mean{}
    {

    }

    template<class T, class E>
    T Basic_File_Stats<T, E>::Get_Min() const noexcept
    {
        return m_min;
    }

    template<class T, class E>
    T Basic_File_Stats<T, E>::Get_Max() const noexcept
    {
        return m_max;
    }

    template<class T, class E>
    T Basic_File_Stats<T, E>::Get_Mean() const noexcept
    {
        return m_mean;
    }

    template<class T, class E>
    [[nodiscard]] int Basic_File_Stats<T, E>::Process()
    {
        m_file->Seek_Beg();
        std::vector<std::future<int>> thread_results(m_number_of_threads);
        for (uint32_t i = 0; i < m_number_of_threads; ++i)
        {
            thread_results[i] = std::async(std::launch::async, &Basic_File_Stats::Worker, this);
        }
        return std::find_if(thread_results.begin(), thread_results.end(), [&](auto& future) {
            return future.get() == -1;
        }) != thread_results.end();
    }

    template<class T, class E>
    void Basic_File_Stats<T, E>::Report_Results(T min, T max, T mean) noexcept
    {
        const std::lock_guard<std::mutex> lock(m_mtx);
        m_min = std::min(m_min, min);
        m_max = std::max(m_max, max);
        m_mean += mean;
    }

    template<class T, class E>
    void Basic_File_Stats<T, E>::Update_Values(T& min, T& max, T& mean, E value) const noexcept
    {
        min = std::min(min, static_cast<T>(value));
        max = std::max(max, static_cast<T>(value));
        mean += static_cast<T>(value) / m_file->Get_Total_Number_Of_Elements();
    }

    template<class T, class E>
    [[nodiscard]] int Basic_File_Stats<T, E>::Worker() noexcept
    {
        T min = std::numeric_limits<T>::max();
        T max = std::numeric_limits<T>::min();
        T mean{};

        while (true)
        {
            const auto [status, count, data] = m_file->Read_Data();
            switch (status)
            {
                case kiv_ppr::File_Reader<E>::Status::OK:
                    for (std::size_t i = 0; i < count; ++i)
                    {
                        Update_Values(min, max, mean, data[i]);
                    }
                    break;
                case File_Reader<E>::Status::ERROR:
                    return -1;
                case File_Reader<E>::Status::EOF_:
                    Report_Results(min, max, mean);
                    return 0;
                case File_Reader<E>::Status::EMPTY:
                    return -2;
            }
        }
    }

    template class Basic_File_Stats<double, double>;
}