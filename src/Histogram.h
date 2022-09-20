#pragma once

#include <cstdint>
#include <vector>
#include <iostream>

#include "FileReader.h"

namespace kiv_ppr
{
    template<class T>
    class Histogram
    {
    public:
        Histogram(uint32_t number_of_slots, T min, T max);
        ~Histogram() = default;

        void Add(T value);
        [[nodiscard]] uint32_t Get_Size() const noexcept;
        std::size_t& operator[](uint32_t index);
        void operator+=(Histogram& other);

        [[nodiscard]] static Histogram<T> Generate_Histogram(File_Reader<T>& file, uint32_t number_of_slots, T min, T max, uint32_t number_of_threads);

        template<class E>
        friend std::ostream& operator<<(std::ostream& out, Histogram<E>& histogram);

    private:
        uint32_t m_number_of_slots;
        std::vector<std::size_t> m_slots;
        T m_min;
        T m_max;
    };
}