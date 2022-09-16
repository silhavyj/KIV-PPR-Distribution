#include <limits>
#include <vector>
#include <future>

#include <spdlog/spdlog.h>

#include "FileStats.h"

namespace kiv_ppr
{
    template<class T, class E>
    File_Stats<T, E>::File_Stats(File_Reader<E>* file)
        : m_file(file), m_min{std::numeric_limits<T>::max()},
                        m_max{std::numeric_limits<T>::min()},
                        m_mean{}
    {

    }

    template<class T, class E>
    T File_Stats<T, E>::Get_Min() const noexcept
    {
        return m_min;
    }

    template<class T, class E>
    T File_Stats<T, E>::Get_Max() const noexcept
    {
        return m_max;
    }

    template<class T, class E>
    T File_Stats<T, E>::Get_Mean() const noexcept
    {
        return m_mean;
    }

    template<class T, class E>
    [[nodiscard]] int File_Stats<T, E>::Process(uint32_t thread_count)
    {
        std::vector<std::future<int>> results(thread_count);
        for (uint32_t i = 0; i < thread_count; ++i)
        {
            results[i] = std::async(std::launch::async, &File_Stats::Worker, this);
        }
        return std::find_if(results.begin(), results.end(), [=](auto& future) {
            return future.get() == -1;
        }) != results.end();
    }

    template<class T, class E>
    [[nodiscard]] Histogram<E> File_Stats<T, E>::Get_Histogram(uint32_t number_of_slots)
    {
        Histogram<E> histogram(number_of_slots, static_cast<E>(m_min), static_cast<E>(m_max));
        m_file->Seek_Beg();
        for (uint32_t i = 0; i < 5; ++i)
        {
            (void)std::async(std::launch::async, &File_Stats::Worker_2, this, &histogram);
        }
        return histogram;
    }

    template<class T, class E>
    void File_Stats<T, E>::Report_2(auto& values, Histogram<E>* histogram) noexcept
    {
        const std::lock_guard<std::mutex> lock(m_mtx);
        for (uint32_t i = 0; i < histogram->Get_Size(); ++i)
        {
            histogram->Insert(values[i]);
        }
    }

    template<class T, class E>
    void File_Stats<T, E>::Worker_2(Histogram<E>* histogram) noexcept
    {
        while (true)
        {
            while (true) {
                const auto [status, count, data] = m_file->Read_Data();
                switch (status) {
                    case kiv_ppr::File_Reader<E>::Flag::OK:
                        Report_2(data, histogram);
                        break;

                    case kiv_ppr::File_Reader<E>::Flag::ERROR:
                        spdlog::error("Could not read data");
                        return;

                    case kiv_ppr::File_Reader<double>::Flag::EOF_:
                        return;
                }
            }
        }
    }

    template<class T, class E>
    void File_Stats<T, E>::Report(T min, T max, T mean) noexcept
    {
        const std::lock_guard<std::mutex> lock(m_mtx);
        m_min = std::min(m_min, min);
        m_max = std::max(m_max, max);
        m_mean += mean;
    }

    template<class T, class E>
    void File_Stats<T, E>::Update(T& min, T& max, T& mean, E value) const noexcept
    {
        min = std::min(min, static_cast<T>(value));
        max = std::max(max, static_cast<T>(value));
        mean += static_cast<T>(value) / m_file->Get_Number_Of_Elements();
    }

    template<class T, class E>
    [[nodiscard]] int File_Stats<T, E>::Worker() noexcept
    {
        T min = std::numeric_limits<T>::max();
        T max = std::numeric_limits<T>::min();
        T mean{};

        while (true)
        {
            const auto [status, count, data] = m_file->Read_Data();
            switch (status)
            {
                case kiv_ppr::File_Reader<E>::Flag::OK:
                    for (std::size_t i = 0; i < count; ++i)
                    {
                        Update(min, max, mean, data[i]);
                    }
                    break;

                case kiv_ppr::File_Reader<E>::Flag::ERROR:
                    spdlog::error("Could not read data");
                    return -1;

                case kiv_ppr::File_Reader<double>::Flag::EOF_:
                    Report(min, max, mean); // All work done
                    return 0;
            }
        }
    }

    template class File_Stats<double, double>;
}