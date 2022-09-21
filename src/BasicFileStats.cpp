#include <limits>
#include <vector>
#include <future>
#include <execution>

#include "BasicFileStats.h"

namespace kiv_ppr
{
    template<class T, class E>
    Basic_File_Stats<T, E>::Basic_File_Stats(File_Reader<E>* file, std::function<bool(E)> num_valid_fce)
        : m_file(file),
          m_min{std::numeric_limits<T>::max()},
          m_max{std::numeric_limits<T>::min()},
          m_num_valid_fce(num_valid_fce),
          m_mean{}
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
    [[nodiscard]] typename Basic_File_Stats<T, E>::Values Basic_File_Stats<T, E>::Get_Values() const noexcept
    {
        return
        {
            Get_Min(),
            Get_Max(),
            Get_Mean()
        };
    }

    template<class T, class E>
    [[nodiscard]] int Basic_File_Stats<T, E>::Process(config::Thread_Config thread_config)
    {
        m_file->Seek_Beg();

        std::vector<std::future<int>> workers(thread_config.number_of_threads);
        for (uint32_t i = 0; i < thread_config.number_of_threads; ++i)
        {
            workers[i] = std::async(std::launch::async, &Basic_File_Stats::Worker, this, thread_config);
        }
        int success = 0;
        std::for_each(std::execution::par, workers.begin(), workers.end(), [&success](auto& worker) {
            success += worker.get();
        });
        return success;
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
    [[nodiscard]] int Basic_File_Stats<T, E>::Worker(const config::Thread_Config& thread_config) noexcept
    {
        T min = std::numeric_limits<T>::max();
        T max = std::numeric_limits<T>::min();
        T mean{};

        while (true)
        {
            const auto [status, count, data] = m_file->Read_Data(thread_config.number_of_elements_per_file_read);
            switch (status)
            {
                case kiv_ppr::File_Reader<E>::Status::OK:
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
                case File_Reader<E>::Status::ERROR:
                    return 1;
                case File_Reader<E>::Status::EOF_:
                    Report_Results(min, max, mean);
                    return 0;
            }
        }
    }

    template class Basic_File_Stats<double, double>;
}