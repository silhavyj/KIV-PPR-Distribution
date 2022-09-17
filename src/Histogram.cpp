#include "Histogram.h"

namespace kiv_ppr
{
    template<class T>
    Histogram<T>::Histogram(uint32_t number_of_slots, T min, T max)
            : m_number_of_slots(number_of_slots),
              m_slots(number_of_slots),
              m_min(min),
              m_max(max)
    {

    }

    template<class T>
    void Histogram<T>::Add(T value)
    {
        static auto slot_size = (m_max - m_min) / m_number_of_slots;
        uint32_t slot_id = (value - m_min) / slot_size;
        ++m_slots[slot_id];
    }

    template<class T>
    uint32_t Histogram<T>::Get_Size() const noexcept
    {
        return m_number_of_slots;
    }

    template<class T>
    std::size_t& Histogram<T>::operator[](uint32_t index)
    {
        return m_slots[index % Get_Size()];
    }

    template<class E>
    std::ostream& operator<<(std::ostream& out, Histogram<E>& histogram)
    {
        uint32_t size = histogram.Get_Size();
        for (uint32_t i = 0; i < size; ++i)
        {
            out << histogram[i] << " ";
        }
        return out;
    }

    template class Histogram<double>;
    template std::ostream& operator<<(std::ostream& out, Histogram<double>& histogram);
}