#pragma once

#include <cstdint>
#include <vector>
#include <iostream>

#include "FileReader.h"

namespace kiv_ppr
{
    template<class T>
    class CHistogram
    {
    public:
        static constexpr uint32_t DEFAULT_NUMBER_OF_SLOTS = 10;

        struct TConfig
        {
            uint32_t number_of_slots;
            T min;
            T max;
        };

    public:
        explicit CHistogram(TConfig config);
        ~CHistogram() = default;

        void Add(T value);
        [[nodiscard]] uint32_t Get_Size() const noexcept;

        [[nodiscard]] size_t& operator[](uint32_t index);
        void operator+=(CHistogram& other);

        template<class E>
        friend std::ostream& operator<<(std::ostream& out, CHistogram<E>& histogram);

    private:
        TConfig m_config;
        std::vector<size_t> m_slots;
    };
}