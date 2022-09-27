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
            uint32_t number_of_intervals;
            double min;
            double max;
        };

    public:
        explicit CHistogram(TConfig config);
        ~CHistogram() = default;

        void Add(double value);
        void Merge_Sparse_Intervals(uint32_t limit);
        [[nodiscard]] uint32_t Get_Number_Of_Intervals() const noexcept;

        [[nodiscard]] size_t& operator[](uint32_t index);
        void operator+=(CHistogram& other);

        [[nodiscard]] size_t Get_Lowest_Frequency() const noexcept;
        [[nodiscard]] size_t Get_Highest_Frequency() const noexcept;
        [[nodiscard]] size_t Get_Sum_Of_All_Frequencies() const noexcept;
        [[nodiscard]] double Get_Mean() const noexcept;
        [[nodiscard]] double Get_Standard_Deviation(double mean) const noexcept;

        friend std::ostream& operator<<(std::ostream& out, CHistogram& histogram);

    private:
        void Merge_Sparse_Intervals_Forward(uint32_t limit);
        void Merge_Sparse_Intervals_Backward(uint32_t limit);

    private:
        std::vector<size_t> m_slots;
        double m_interval_size;
        TConfig m_config;
    };
}