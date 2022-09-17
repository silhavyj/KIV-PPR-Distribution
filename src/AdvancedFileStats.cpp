#include <limits>
#include <vector>
#include <future>

#include "AdvancedFileStats.h"

namespace kiv_ppr
{
    template<class T, class E>
    Advanced_File_Stats<T, E>::Advanced_File_Stats(File_Reader<E>* file, typename Basic_File_Stats<T, E>::Values values, uint32_t number_of_threads)
        : m_file(file),
          m_values(values),
          m_number_of_threads(number_of_threads)
    {

    }

    template<class T, class E>
    [[nodiscard]] std::shared_ptr<Histogram<T>> Advanced_File_Stats<T, E>::Get_Histogram() noexcept
    {
        return m_histogram;
    }

    template<class T, class E>
    [[nodiscard]] int Advanced_File_Stats<T, E>::Process()
    {
        m_file->Seek_Beg();
        m_histogram = std::make_shared<Histogram<T>>(Histogram<T>::DEFAULT_NUMBER_OF_SLOTS, m_values.min, m_values.max);

        std::vector<std::future<int>> thread_results(m_number_of_threads);
        for (uint32_t i = 0; i < m_number_of_threads; ++i)
        {
            thread_results[i] = std::async(std::launch::async, &Advanced_File_Stats::Worker, this);
        }
        return std::find_if(thread_results.begin(), thread_results.end(), [&](auto& future) {
            return future.get() < 0;
        }) == thread_results.end();
    }

    template<class T, class E>
    [[nodiscard]] int Advanced_File_Stats<T, E>::Worker() noexcept
    {
        Histogram<T> histogram(Histogram<T>::DEFAULT_NUMBER_OF_SLOTS, m_values.min, m_values.max);

        while (true)
        {
            const auto [status, count, data] = m_file->Read_Data();
            switch (status)
            {
                case kiv_ppr::File_Reader<E>::Status::OK:
                    for (std::size_t i = 0; i < count; ++i)
                    {
                        histogram.Add(static_cast<T>(data[i]));
                    }
                    break;
                case File_Reader<E>::Status::ERROR:
                    return -1;
                case File_Reader<E>::Status::EOF_:
                    Report_Results(histogram);
                    return 0;
                case File_Reader<E>::Status::EMPTY:
                    return -2;
            }
        }
    }

    template<class T, class E>
    void Advanced_File_Stats<T, E>::Report_Results(Histogram<T>& histogram) noexcept
    {
        const std::lock_guard<std::mutex> lock(m_mtx);
        static auto size = m_histogram->Get_Size();

        for (uint32_t i = 0; i < size; ++i)
        {
            m_histogram->operator[](i) += histogram[i];
        }
    }

    template class Advanced_File_Stats<double, double>;
}