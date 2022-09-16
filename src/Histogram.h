#pragma once

#include <cstdint>
#include <vector>

namespace kiv_ppr
{
    template<class T>
    class Histogram
    {
    public:
        Histogram(uint32_t number_of_slots, T min, T max);
        ~Histogram() = default;

        void Insert(T value);
        [[nodiscard]] std::size_t Get_Size() const noexcept;
        std::size_t& operator[](uint32_t index);

    private:
        uint32_t m_number_of_slots;
        std::vector<std::size_t> m_slots;
        T m_min;
        T m_max;

    };
}