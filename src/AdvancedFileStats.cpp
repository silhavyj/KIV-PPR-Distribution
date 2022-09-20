#include <vector>
#include <future>
#include <cmath>

#include "AdvancedFileStats.h"
#include "Config.h"

namespace kiv_ppr
{
    template<class T, class E>
    Advanced_File_Stats<T, E>::Advanced_File_Stats(File_Reader<E> *file, std::function<bool(E)> num_valid_fce, typename Basic_File_Stats<T, E>::Values basic_values, typename Histogram<T>::Config histogram_config)
        : m_file(file),
          m_num_valid_fce(num_valid_fce),
          m_basic_values(basic_values),
          m_histogram_config(histogram_config),
          m_standard_deviation{},
          m_histogram{std::make_shared<Histogram<T>>(histogram_config)}
    {

    }

    template<class T, class E>
    [[nodiscard]] T Advanced_File_Stats<T, E>::Get_Standard_Deviation() const noexcept
    {
        return m_standard_deviation;
    }

    template<class T, class E>
    [[nodiscard]] std::shared_ptr<Histogram<T>> Advanced_File_Stats<T, E>::Get_Histogram() const noexcept
    {
        return m_histogram;
    }

    template<class T, class E>
    [[nodiscard]] typename Advanced_File_Stats<T, E>::Values Advanced_File_Stats<T, E>::Get_Values() const noexcept
    {
        return
        {
            Get_Standard_Deviation(),
            Get_Histogram()
        };
    }

    template<class T, class E>
    [[nodiscard]] int Advanced_File_Stats<T, E>::Process(uint32_t number_of_threads)
    {
        m_file->Seek_Beg();

        std::vector<std::future<int>> workers(number_of_threads);
        for (uint32_t i = 0; i < number_of_threads; ++i)
        {
            workers[i] = std::async(std::launch::async, &Advanced_File_Stats::Worker, this);
        }
        for (auto& worker : workers)
        {
            if (worker.get() < 0)
            {
                return 1;
            }
        }
        m_standard_deviation = std::sqrt(m_standard_deviation);
        return 0;
    }

    template<class T, class E>
    void Advanced_File_Stats<T, E>::Report_Results(T standard_deviation, Histogram<T> &histogram) noexcept
    {
        const std::lock_guard<std::mutex> lock(m_mtx);

        m_standard_deviation += standard_deviation;
        m_histogram->operator+=(histogram);
    }

    template<class T, class E>
    [[nodiscard]] int Advanced_File_Stats<T, E>::Worker() noexcept
    {
        Histogram<T> histogram(m_histogram_config);
        T standard_deviation{};
        T tmp_value1;
        T tmp_value2;

        while (true)
        {
            const auto [status, count, data] = m_file->Read_Data(kiv_ppr::config::NUMBER_OF_ELEMENTS_PER_READ);
            switch (status)
            {
                case kiv_ppr::File_Reader<E>::Status::OK:
                    for (std::size_t i = 0; i < count; ++i)
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
                case File_Reader<E>::Status::ERROR:
                    return -1;
                case File_Reader<E>::Status::EOF_:
                    Report_Results(standard_deviation, histogram);
                    return 0;
            }
        }
    }

    template class Advanced_File_Stats<double, double>;
}