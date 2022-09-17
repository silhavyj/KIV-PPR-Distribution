#include <cmath>

#include <spdlog/spdlog.h>

#include "Histogram.h"

namespace kiv_ppr
{
    template<class T>
    Histogram<T>::Histogram(uint32_t number_of_slots, T min, T max)
        : m_number_of_slots(number_of_slots), m_slots(number_of_slots), m_min(min), m_max(max)
    {

    }

    template<class T>
    void Histogram<T>::Insert(T value)
    {
        static auto slot_size = (m_max - m_min) / m_number_of_slots;

        if (value == m_max)
        {
            ++m_slots[m_number_of_slots - 1];
        }
        else
        {
            uint32_t slot_id = value / slot_size;
            ++m_slots[slot_id];
        }
    }

    template<class T>
    std::size_t Histogram<T>::Get_Size() const noexcept
    {
        return m_number_of_slots;
    }

    template<class T>
    std::size_t& Histogram<T>::operator[](uint32_t index)
    {
        return m_slots[index % Get_Size()];
    }

    template class Histogram<double>;
}