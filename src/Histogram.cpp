#include <vector>
#include <future>
#include <iostream>
#include <numeric>
#include <cmath>

#include "Histogram.h"

namespace kiv_ppr
{
    CHistogram::CHistogram(TConfig config)
        : m_slots(config.number_of_intervals + 1),
          m_interval_size((config.max - config.min) / config.number_of_intervals),
          m_config(config)
    {

    }

    void CHistogram::Add(double value)
    {
        const auto slot_id = static_cast<size_t>((value - m_config.min) / m_interval_size);
        ++m_slots[slot_id];
    }

    void CHistogram::Merge_Sparse_Intervals(uint32_t limit)
    {
        Merge_Sparse_Intervals_Forward(limit);
        Merge_Sparse_Intervals_Backward(limit);
    }

    void CHistogram::Merge_Sparse_Intervals_Forward(uint32_t limit)
    {
        auto it = m_slots.begin();
        while (it != m_slots.end() - 1)
        {
            if (*it < limit)
            {
                *(it + 1) += *it;
                if (it == m_slots.begin())
                {
                    m_config.min += m_interval_size;
                }
                m_slots.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    void CHistogram::Merge_Sparse_Intervals_Backward(uint32_t limit)
    {
        auto it = m_slots.end() - 1;
        while (it != m_slots.begin())
        {
            if (*it < limit)
            {
                *(it - 1) += *it;
                if (it == m_slots.end() - 1)
                {
                    m_config.max -= m_interval_size;
                }
                m_slots.erase(it);
            }
            --it;
        }
    }

    [[nodiscard]] uint32_t CHistogram::Get_Number_Of_Intervals() const noexcept
    {
        return m_slots.size();
    }

    [[nodiscard]] size_t& CHistogram::operator[](uint32_t index)
    {
        return m_slots[index % Get_Number_Of_Intervals()];
    }

    void CHistogram::operator+=(CHistogram& other)
    {
        uint32_t size = std::min(Get_Number_Of_Intervals(), other.Get_Number_Of_Intervals());
        for (uint32_t i = 0; i < size; ++i)
        {
            (*this)[i] += other[i];
        }
    }

    [[nodiscard]] size_t CHistogram::Get_Lowest_Frequency() const noexcept
    {
        return *std::min_element(m_slots.begin(), m_slots.end());
    }

    [[nodiscard]] size_t CHistogram::Get_Highest_Frequency() const noexcept
    {
        return *std::max_element(m_slots.begin(), m_slots.end());
    }

    [[nodiscard]] size_t CHistogram::Get_Sum_Of_All_Frequencies() const noexcept
    {
        return std::accumulate(m_slots.begin(), m_slots.end(), 0);
    }

    [[nodiscard]] double CHistogram::Get_Min() const noexcept
    {
        return m_config.min;
    }

    [[nodiscard]] double CHistogram::Get_Max() const noexcept
    {
        return m_config.max;
    }

    [[nodiscard]] double CHistogram::Get_Interval_Size() const noexcept
    {
        return m_interval_size;
    }

    [[nodiscard]] double CHistogram::Get_Mean() const noexcept
    {
        double mean = 0.0;
        double middle_value;
        const size_t count = Get_Sum_Of_All_Frequencies();
        const uint32_t number_of_intervals = Get_Number_Of_Intervals();

        for (size_t i = 0; i < number_of_intervals; ++i)
        {
            middle_value = m_config.min + (i * m_interval_size) + (m_interval_size / 2.0);
            mean += (middle_value / count) * m_slots[i];
        }
        return mean;
    }

    [[nodiscard]] double CHistogram::Get_Standard_Deviation(double mean) const noexcept
    {
        double sd = 0.0;
        double middle_value;
        double delta;
        const size_t count = Get_Sum_Of_All_Frequencies();
        const uint32_t number_of_intervals = Get_Number_Of_Intervals();

        for (size_t i = 0; i < number_of_intervals; ++i)
        {
            middle_value = m_config.min + (i * m_interval_size) + (m_interval_size / 2.0);
            delta = mean - middle_value;
            sd += (delta / (count - 1)) * delta * m_slots[i];
        }
        return std::sqrt(sd);
    }

    std::ostream& operator<<(std::ostream& out, CHistogram& histogram)
    {
        uint32_t size = histogram.Get_Number_Of_Intervals();
        for (uint32_t i = 0; i < size; ++i)
        {
            out << histogram[i] << " ";
        }
        return out;
    }
}