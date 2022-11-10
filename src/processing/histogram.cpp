#include "histogram.h"

namespace kiv_ppr
{
    CHistogram::CHistogram(TParams params)
        : m_intervals(params.number_of_intervals + 1, 0),
          m_interval_size((params.max - params.min) / static_cast<double>(params.number_of_intervals)),
          m_params(params),
          m_count{}
    {

    }

    void CHistogram::Add(double value) noexcept
    {
        // Add the value into its corresponding bin (interval).
        const auto slot_id = static_cast<size_t>((value - m_params.min) / m_interval_size);
        ++m_intervals[slot_id];

        // Increment the number of values inserted into the histogram.
        ++m_count;
    }

    size_t CHistogram::Get_Number_Of_Intervals() const noexcept
    {
        return m_intervals.size();
    }

    bool CHistogram::Add(size_t index, size_t value) noexcept
    {
        if (index < m_intervals.size())
        {
            // Update the current interval (bin).
            m_intervals[index] += value;
            
            // Update the number of values stored in the histogram.
            m_count += value;
            return true;
        }

        // Index is out of range.
        return false;
    }

    double CHistogram::Get_Min() const noexcept
    {
        return m_params.min;
    }

    double CHistogram::Get_Interval_Size() const noexcept
    {
        return m_interval_size;
    }

    size_t CHistogram::Get_Total_Count() const noexcept
    {
        return m_count;
    }

    size_t& CHistogram::operator[](size_t index) noexcept
    {
        return m_intervals[index % Get_Number_Of_Intervals()];
    }

    void CHistogram::operator+=(CHistogram& other) noexcept
    {
        // Make sure we do not overflow (take the minimum of the two histograms).
        const size_t size = std::min(Get_Number_Of_Intervals(), other.Get_Number_Of_Intervals());

        // Merge the histograms.
        for (size_t i = 0; i < size; ++i)
        {
            // Merge the current interval (bin).
            (*this)[i] += other[i];

            // Update the number of values stored in the histogram.
            m_count += other[i];
        }
    }

    std::ostream& operator<<(std::ostream& out, CHistogram& histogram)
    {
        const size_t size = histogram.Get_Number_Of_Intervals();
        size_t sum{};
        for (size_t i = 0; i < size; ++i)
        {
            out << i << ": " << histogram[i] << '\n';
            sum += histogram[i];
        }
        out << "sum: " << sum;
        return out;
    }
}

// EOF