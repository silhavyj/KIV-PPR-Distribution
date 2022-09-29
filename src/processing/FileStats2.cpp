#include <future>
#include <cmath>

#include "FileStats2.h"
#include "../utils/Utils.h"

namespace kiv_ppr
{
    CFile_Stats_2::CFile_Stats_2(CFile_Reader<double>* file,
                                 std::function<bool(double)> is_valid_number,
                                 typename CFile_Stats_1::TValues basic_values)
        : m_file(file),
          m_is_valid_number(std::move(is_valid_number)),
          m_basic_values(basic_values),
          m_values{},
          m_histogram_params{}
    {
        m_histogram_params = {
            m_basic_values.min,
            m_basic_values.max,
            Calculate_Number_Of_Intervals(m_basic_values.count)
        };
        m_values.histogram = std::make_shared<CHistogram>(m_histogram_params);
    }

    double CFile_Stats_2::Get_Var() const noexcept
    {
        return m_values.var;
    }

    double CFile_Stats_2::Get_SD() const noexcept
    {
        return m_values.sd;
    }

    std::shared_ptr<CHistogram> CFile_Stats_2::Get_Histogram() const noexcept
    {
        return m_values.histogram;
    }

    typename CFile_Stats_2::TValues CFile_Stats_2::Get_Values() const noexcept
    {
        return m_values;
    }

    int CFile_Stats_2::Run(config::TThread_Params* thread_config)
    {
        m_file->Seek_Beg();
        std::vector<std::future<int>> workers(thread_config->number_of_threads);
        for (auto& worker : workers)
        {
            worker = std::async(std::launch::async, &CFile_Stats_2::Worker, this, thread_config);
        }

        int return_values = 0;
        for (auto& worker : workers)
        {
            return_values += worker.get();
        }

        m_values.sd = std::sqrt(m_values.var);
        if (return_values != 0)
        {
            return 1;
        }
        return 0;
    }

    void CFile_Stats_2::Report_Worker_Results(const TValues& values)
    {
        const std::lock_guard<std::mutex> lock(m_mtx);
        m_values.var += values.var;
        m_values.histogram->operator+=(*values.histogram);
    }

    int CFile_Stats_2::Worker(config::TThread_Params* thread_config)
    {
        TValues local_values {
            0.0,
            0,
            std::make_shared<CHistogram>(m_histogram_params)
        };
        double delta;
        double tmp_value;

        while (true)
        {
            const auto [status, count, data] = m_file->Read_Data(thread_config->number_of_elements_per_file_read);
            switch (status)
            {
                case kiv_ppr::CFile_Reader<double>::NRead_Status::OK:
                    for (auto i = 0; i < count; ++i)
                    {
                        const double value = data[i];
                        if (m_is_valid_number(value))
                        {
                            delta = value - m_basic_values.mean;
                            tmp_value = delta;
                            delta /= static_cast<double>(m_basic_values.count - 1);
                            delta *= tmp_value;

                            local_values.var += delta;
                            local_values.histogram->Add(value);
                        }
                    }
                    break;

                case CFile_Reader<double>::NRead_Status::ERROR:
                    return 1;

                case CFile_Reader<double>::NRead_Status::EOF_:
                    Report_Worker_Results(local_values);
                    return 0;
            }
        }
    }

    size_t CFile_Stats_2::Calculate_Number_Of_Intervals(size_t n)
    {
        static constexpr size_t MAX_LIMIT = 1024 * 1024 * 50; // 50 MB
        return std::min(static_cast<size_t>(2.82 * std::pow(n, 2.0 / 5.0)), MAX_LIMIT);
    }
}