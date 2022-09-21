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
        static constexpr uint32_t DEFAULT_NUMBER_OF_SLOTS = 10;

        struct Config
        {
            uint32_t number_of_slots;
            T min;
            T max;
        };

    public:
        explicit Histogram(Config config);
        ~Histogram() = default;

        void Add(T value);
        [[nodiscard]] uint32_t Get_Size() const noexcept;

        [[nodiscard]] std::size_t& operator[](uint32_t index);
        void operator+=(Histogram& other);

        template<class E>
        friend std::ostream& operator<<(std::ostream& out, Histogram<E>& histogram);

    private:
        Config m_config;
        std::vector<std::size_t> m_slots;
    };
}