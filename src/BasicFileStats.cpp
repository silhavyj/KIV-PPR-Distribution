#include <limits>
#include <vector>
#include <future>

#include <spdlog/spdlog.h>

#include "BasicFileStats.h"

namespace kiv_ppr
{
    template<class T, class E>
    Basic_File_Stats<T, E>::Basic_File_Stats(File_Reader<E>* file, uint32_t number_of_threads)
        : m_file(file),
          m_number_of_threads(number_of_threads),
          m_min{std::numeric_limits<T>::max()},
          m_max{std::numeric_limits<T>::min()},
          m_mean{},
          m_median{}
    {

    }

    template<class T, class E>
    [[nodiscard]] T Basic_File_Stats<T, E>::Get_Min() const noexcept
    {
        return m_min;
    }

    template<class T, class E>
    [[nodiscard]] T Basic_File_Stats<T, E>::Get_Max() const noexcept
    {
        return m_max;
    }

    template<class T, class E>
    [[nodiscard]] T Basic_File_Stats<T, E>::Get_Mean() const noexcept
    {
        return m_mean;
    }

    template<class T, class E>
    [[nodiscard]] T Basic_File_Stats<T, E>::Get_Median() const noexcept
    {
        return m_median;
    }

    template<class T, class E>
    [[nodiscard]] typename Basic_File_Stats<T, E>::Values Basic_File_Stats<T, E>::Get_Values() const noexcept
    {
        return
        {
            Get_Min(),
            Get_Max(),
            Get_Mean(),
            Get_Median()
        };
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
        auto invalid_result = std::find_if(thread_results.begin(), thread_results.end(), [&](auto& future) {
            return future.get() < 0;
        });
        if (!m_median_finder.Is_Empty())
        {
            m_median = m_median_finder.Get_Median();
        }
        return invalid_result != thread_results.end();
    }

    template<class T, class E>
    void Basic_File_Stats<T, E>::Report_Results(T min, T max, T mean, Stream_Median_Finder<T>& median_finder) noexcept
    {
        const std::lock_guard<std::mutex> lock(m_mtx);

        m_min = std::min(m_min, min);
        m_max = std::max(m_max, max);
        m_mean += mean;

        while (!median_finder.Is_Empty())
        {
            m_median_finder.Add_Value(median_finder.Pop_Value());
        }
    }

    template<class T, class E>
    [[nodiscard]] int Basic_File_Stats<T, E>::Worker() noexcept
    {
        T min = std::numeric_limits<T>::max();
        T max = std::numeric_limits<T>::min();
        T mean{};
        Stream_Median_Finder<T> median_finder;

        while (true)
        {
            const auto [status, count, data] = m_file->Read_Data();
            switch (status)
            {
                case kiv_ppr::File_Reader<E>::Status::OK:
                    for (std::size_t i = 0; i < count; ++i)
                    {
                        min = std::min(min, static_cast<T>(data[i]));
                        max = std::max(max, static_cast<T>(data[i]));
                        mean += static_cast<T>(data[i]) / m_file->Get_Total_Number_Of_Elements();
                        median_finder.Add_Value(static_cast<T>(data[i]));
                    }
                    break;
                case File_Reader<E>::Status::ERROR:
                    return -1;
                case File_Reader<E>::Status::EOF_:
                    Report_Results(min, max, mean, median_finder);
                    return 0;
            }
        }
    }

    template class Basic_File_Stats<double, double>;
}