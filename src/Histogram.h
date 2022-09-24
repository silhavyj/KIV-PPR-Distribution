#pragma once

#include <cstdint>
#include <vector>
#include <iostream>

#include "FileReader.h"

namespace kiv_ppr
{
    class CHistogram
    {
    public:
        struct TConfig
        {
            uint32_t number_of_slots;
            double min;
            double max;
        };

    public:
        explicit CHistogram(TConfig config);
        ~CHistogram() = default;

        void Add(double value);
        [[nodiscard]] uint32_t Get_Size() const noexcept;

        [[nodiscard]] size_t& operator[](uint32_t index);
        void operator+=(CHistogram& other);

        [[nodiscard]] size_t Get_Lowest_Frequency() const noexcept;
        [[nodiscard]] size_t Get_Highest_Frequency() const noexcept;

        friend std::ostream& operator<<(std::ostream& out, CHistogram& histogram);

    private:
        TConfig m_config;
        std::vector<size_t> m_slots;
    };
}