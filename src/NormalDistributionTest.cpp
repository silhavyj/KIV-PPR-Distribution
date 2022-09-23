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
          m_category1{},
          m_category2{},
          m_category3{}
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

        const size_t N = m_file->Get_Total_Number_Of_Valid_Elements();

        std::cout << "68 -> " << (static_cast<double>(m_category1) / N * 100.0) << "\n";
        std::cout << "95 -> " << (static_cast<double>(m_category2) / N * 100.0) << "\n";
        std::cout << "99.7 -> " << (static_cast<double>(m_category3) / N * 100.0) << "\n";

        return 0;
    }

    template<class T, class E>
    void CNormal_Distribution_Test<T, E>::Report_Results(size_t category1, size_t category2, size_t category3)
    {
        const std::lock_guard<std::mutex> lock(m_mtx);
        m_category1 += category1;
        m_category2 += category2;
        m_category3 += category3;
    }

    template<class T, class E>
    [[nodiscard]] int CNormal_Distribution_Test<T, E>::Worker(const config::TThread_Config* thread_config) noexcept
    {
        size_t category1{};
        size_t category2{};
        size_t category3{};

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

                            if (value >= m_mean - m_SD && value <= m_mean + m_SD)
                            {
                                category1++;
                            }
                            if (value >= m_mean - 2 * m_SD && value <= m_mean + 2 * m_SD)
                            {
                                category2++;
                            }
                            if (value >= m_mean - 3 * m_SD && value <= m_mean + 3 * m_SD)
                            {
                                category3++;
                            }
                        }
                    }
                    break;
                case CFile_Reader<E>::NStatus::ERROR:
                    return 1;
                case CFile_Reader<E>::NStatus::EOF_:
                    Report_Results(category1, category2, category3);
                    return 0;
            }
        }
    }

    template class CNormal_Distribution_Test<double, double>;
}