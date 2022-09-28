#include "Histogram.h"

namespace kiv_ppr
{
    CHistogram::CHistogram(TParams params)
        : m_intervals(params.number_of_intervals + 1),
          m_interval_size((params.max - params.min) / static_cast<double>(params.number_of_intervals)),
          m_params(params),
          m_count{}
    {

    }

    void CHistogram::Add(double value)
    {
        const auto slot_id = static_cast<size_t>((value - m_params.min) / m_interval_size);
        ++m_intervals[slot_id];
        ++m_count;
    }

    void CHistogram::Merge_Sparse_Intervals(size_t min_number)
    {
        Merge_Sparse_Intervals_Forward(min_number);
        Merge_Sparse_Intervals_Backward(min_number);
    }

    size_t CHistogram::Get_Number_Of_Intervals() const noexcept
    {
        return m_intervals.size();
    }

    double CHistogram::Get_Min() const noexcept
    {
        return m_params.min;
    }

    double CHistogram::Get_Max() const noexcept
    {
        return m_params.max;
    }

    double CHistogram::Get_Interval_Size() const noexcept
    {
        return m_interval_size;
    }

    size_t CHistogram::Get_Total_Count() const
    {
        return m_count;
    }

    size_t& CHistogram::operator[](uint32_t index)
    {
        return m_intervals[index % Get_Number_Of_Intervals()];
    }

    void CHistogram::operator+=(CHistogram& other)
    {
        const uint32_t size = std::min(Get_Number_Of_Intervals(), other.Get_Number_Of_Intervals());
        for (uint32_t i = 0; i < size; ++i)
        {
            (*this)[i] += other[i];
            m_count += other[i];
        }
    }

    void CHistogram::Merge_Sparse_Intervals_Forward(uint32_t min_number)
    {
        auto it = m_intervals.begin();
        while (it != m_intervals.end() - 1)
        {
            if (*it < min_number)
            {
                *(it + 1) += *it;
                if (it == m_intervals.begin())
                {
                    m_params.min += m_interval_size;
                }
                m_intervals.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    void CHistogram::Merge_Sparse_Intervals_Backward(uint32_t min_number)
    {
        auto it = m_intervals.end() - 1;
        while (it != m_intervals.begin())
        {
            if (*it < min_number)
            {
                *(it - 1) += *it;
                if (it == m_intervals.end() - 1)
                {
                    m_params.max -= m_interval_size;
                }
                m_intervals.erase(it);
            }
            --it;
        }
    }

    std::ostream& operator<<(std::ostream& out, CHistogram& histogram)
    {
        const uint32_t size = histogram.Get_Number_Of_Intervals();
        for (uint32_t i = 0; i < size; ++i)
        {
            out << histogram[i] << " ";
        }
        return out;
    }
}