#include <vector>
#include <future>

#include "Config.h"
#include "Histogram.h"

namespace kiv_ppr
{
    template<class T>
    Histogram<T>::Histogram(Config config)
        : m_config(config),
          m_slots(config.number_of_slots)
    {

    }

    template<class T>
    void Histogram<T>::Add(T value)
    {
        static auto slot_size = (m_config.max - m_config.min) / m_config.number_of_slots;
        uint32_t slot_id = (value - m_config.min) / slot_size;
        ++m_slots[slot_id];
    }

    template<class T>
    [[nodiscard]] uint32_t Histogram<T>::Get_Size() const noexcept
    {
        return m_config.number_of_slots;
    }

    template<class T>
    [[nodiscard]] std::size_t& Histogram<T>::operator[](uint32_t index)
    {
        return m_slots[index % Get_Size()];
    }

    template<class T>
    void Histogram<T>::operator+=(Histogram<T>& other)
    {
        uint32_t size = std::min(Get_Size(), other.Get_Size());
        for (uint32_t i = 0; i < size; ++i)
        {
            (*this)[i] += other[i];
        }
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