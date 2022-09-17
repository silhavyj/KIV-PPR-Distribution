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
    void Basic_File_Stats<T, E>::Report_Results(T min, T max, T mean, Stream_Median_Finder& median_finder) noexcept
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
        Stream_Median_Finder median_finder;

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
                        median_finder.Add_Value(data[i]);
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

    template<class T, class E>
    void Basic_File_Stats<T, E>::Stream_Median_Finder::Add_Value(T value)
    {
        m_left_half.push(value);

        if ((!m_left_half.empty() && !m_right_half.empty()) && (m_left_half.top() > m_right_half.top()))
        {
            m_right_half.push(m_left_half.top());
            m_left_half.pop();
        }
        if (m_left_half.size() > m_right_half.size() + 1)
        {
            m_right_half.push(m_left_half.top());
            m_left_half.pop();
        }
        if (m_left_half.size() + 1 < m_right_half.size())
        {
            m_left_half.push(m_right_half.top());
            m_right_half.pop();
        }
    }

    template<class T, class E>
    T Basic_File_Stats<T, E>::Stream_Median_Finder::Get_Median() const
    {
        if (m_left_half.size() > m_right_half.size())
        {
            return m_left_half.top();
        }
        else if (m_right_half.size() > m_left_half.size())
        {
            return m_right_half.top();
        }
        return (m_left_half.top() + m_right_half.top()) / 2.0;
    }

    template<class T, class E>
    bool Basic_File_Stats<T, E>::Stream_Median_Finder::Is_Empty() const
    {
        return m_left_half.empty() && m_right_half.empty();
    }

    template<class T, class E>
    T Basic_File_Stats<T, E>::Stream_Median_Finder::Pop_Value()
    {
        T value{};
        if (!m_left_half.empty())
        {
            value = m_left_half.top();
            m_left_half.pop();
        }
        else if (!m_right_half.empty())
        {
            value = m_right_half.top();
            m_right_half.pop();
        }
        return value;
    }

    template class Basic_File_Stats<double, double>;
}