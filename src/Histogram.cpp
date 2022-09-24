#include <vector>
#include <future>
#include <iostream>
#include <cmath>

#include "Histogram.h"

namespace kiv_ppr
{
    CHistogram::CHistogram(TConfig config)
        : m_config(config),
          m_slots(config.number_of_slots + 1)
    {

    }

    void CHistogram::Add(double value)
    {
        static auto slot_size = (m_config.max - m_config.min) / m_config.number_of_slots;
        const auto slot_id = static_cast<size_t>((value - m_config.min) / slot_size);
        ++m_slots[slot_id];
    }

    [[nodiscard]] uint32_t CHistogram::Get_Size() const noexcept
    {
        return m_config.number_of_slots;
    }

    [[nodiscard]] size_t& CHistogram::operator[](uint32_t index)
    {
        return m_slots[index % Get_Size()];
    }

    void CHistogram::operator+=(CHistogram& other)
    {
        uint32_t size = std::min(Get_Size(), other.Get_Size());
        for (uint32_t i = 0; i < size; ++i)
        {
            (*this)[i] += other[i];
        }
    }

    [[nodiscard]] size_t CHistogram::Get_Lowest_Frequency() const noexcept
    {
        return *std::min_element(m_slots.begin(), m_slots.end() - 1);
    }

    [[nodiscard]] size_t CHistogram::Get_Highest_Frequency() const noexcept
    {
        return *std::max_element(m_slots.begin(), m_slots.end() - 1);
    }

    std::ostream& operator<<(std::ostream& out, CHistogram& histogram)
    {
        uint32_t size = histogram.Get_Size();
        for (uint32_t i = 0; i < size; ++i)
        {
            out << histogram[i] << " ";
        }
        return out;
    }
}