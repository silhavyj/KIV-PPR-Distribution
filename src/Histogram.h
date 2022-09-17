#pragma once

#include <cstdint>
#include <vector>
#include <iostream>

namespace kiv_ppr
{
    template<class T>
    class Histogram
    {
    public:
        static constexpr uint32_t DEFAULT_NUMBER_OF_SLOTS = 10;

    public:
        Histogram(uint32_t number_of_slots, T min, T max);
        ~Histogram() = default;

        void Add(T value);
        [[nodiscard]] uint32_t Get_Size() const noexcept;
        std::size_t& operator[](uint32_t index);

        template<class E>
        friend std::ostream& operator<<(std::ostream& out, Histogram<E>& histogram);

    private:
        uint32_t m_number_of_slots;
        std::vector<std::size_t> m_slots;
        T m_min;
        T m_max;
    };
}