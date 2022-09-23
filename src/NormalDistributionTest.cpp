#include <future>
#include <iostream>

#include "NormalDistributionTest.h"

namespace kiv_ppr
{
    template<class T, class E>
    CNormal_Distribution_Test<T, E>::CNormal_Distribution_Test(CFile_Reader<E>* file, std::function<bool(E)> num_valid_fce, T mean, T SD)
        : m_file(file),
          m_num_valid_fce(num_valid_fce),
          m_mean(mean),
          m_SD(SD),
          m_category_68{},
          m_category_95{},
          m_category_99_7{}
    {

    }

    template<class T, class E>
    [[nodiscard]] int CNormal_Distribution_Test<T, E>::Process(config::TThread_Config thread_config)
    {
        m_file->Seek_Beg();

        std::vector<std::future<int>> workers(thread_config.number_of_threads);
        for (auto& worker : workers)
        {
            worker = std::async(std::launch::async, &CNormal_Distribution_Test::Worker, this, &thread_config);
        }

        int ret_values = 0;
        for (auto& worker : workers)
        {
            ret_values += worker.get();
        }
        if (ret_values != 0)
        {
            return 1;
        }
        return 0;
    }

    template<class T, class E>
    void CNormal_Distribution_Test<T, E>::Report_Results(size_t category_68, size_t category_95, size_t category_99_7)
    {
        const std::lock_guard<std::mutex> lock(m_mtx);
        m_category_68 += category_68;
        m_category_95 += category_95;
        m_category_99_7 += category_99_7;
    }

    template<class T, class E>
    bool CNormal_Distribution_Test<T, E>::Is_Normal_Distribution(double tolerance)
    {
        return Is_Within_Tolerance(0.68, Get_Category_68(), tolerance) &&
               Is_Within_Tolerance(0.95, Get_Category_95(), tolerance) &&
               Is_Within_Tolerance(0.997, Get_Category_99_7(), tolerance);
    }

    template<class T, class E>
    [[nodiscard]] bool CNormal_Distribution_Test<T, E>::Is_Within_Tolerance(double expected, double actual, double percentage) noexcept
    {
        const double tolerance = expected * percentage;
        return (actual >= (expected - tolerance)) && (actual <= (expected + tolerance));
    }

    template<class T, class E>
    [[nodiscard]] double CNormal_Distribution_Test<T, E>::Get_Category_68() const noexcept
    {
        return static_cast<double>(m_category_68) / m_file->Get_Total_Number_Of_Valid_Elements();
    }

    template<class T, class E>
    [[nodiscard]] double CNormal_Distribution_Test<T, E>::Get_Category_95() const noexcept
    {
        return static_cast<double>(m_category_95) / m_file->Get_Total_Number_Of_Valid_Elements();
    }

    template<class T, class E>
    [[nodiscard]] double CNormal_Distribution_Test<T, E>::Get_Category_99_7() const noexcept
    {
        return static_cast<double>(m_category_99_7) / m_file->Get_Total_Number_Of_Valid_Elements();
    }

    template<class T, class E>
    [[nodiscard]] int CNormal_Distribution_Test<T, E>::Worker(const config::TThread_Config* thread_config) noexcept
    {
        size_t category_68{};
        size_t category_95{};
        size_t category_99_7{};

        T min_val_category_68 = m_mean - m_SD;
        T max_val_category_68 = m_mean + m_SD;

        T min_val_category_95 = m_mean - 2 * m_SD;
        T max_val_category_95 = m_mean + 2 * m_SD;

        T min_val_category_99_7 = m_mean - 3 * m_SD;
        T max_val_category_99_7 = m_mean + 3 * m_SD;

        while (true)
        {
            const auto [status, count, data] = m_file->Read_Data(thread_config->number_of_elements_per_file_read);
            switch (status)
            {
                case kiv_ppr::CFile_Reader<E>::NStatus::OK:
                    for (size_t i = 0; i < count; ++i)
                    {
                        if (m_num_valid_fce(data[i]))
                        {
                            const T value = static_cast<T>(data[i]);

                            category_68 += value >= min_val_category_68 && value <= max_val_category_68;
                            category_95 += value >= min_val_category_95 && value <= max_val_category_95;
                            category_99_7 += value >= min_val_category_99_7 && value <= max_val_category_99_7;
                        }
                    }
                    break;
                case CFile_Reader<E>::NStatus::ERROR:
                    return 1;
                case CFile_Reader<E>::NStatus::EOF_:
                    Report_Results(category_68, category_95, category_99_7);
                    return 0;
            }
        }
    }

    template class CNormal_Distribution_Test<double, double>;
}