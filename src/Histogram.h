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
        struct Config
        {
            uint32_t number_of_slots;
            T min_value;
            T max_value;
        };

    public:
        explicit Histogram(Config config);
        ~Histogram() = default;

        void Add(T value);
        [[nodiscard]] uint32_t Get_Size() const noexcept;
        std::size_t& operator[](uint32_t index);
        void operator+=(Histogram& other);

        [[nodiscard]] static Histogram<T> Generate_Histogram(File_Reader<T>& file, Config histogram_config, uint32_t number_of_threads);

        template<class E>
        friend std::ostream& operator<<(std::ostream& out, Histogram<E>& histogram);

    private:
        Config m_config;
        std::vector<std::size_t> m_slots;
    };
}