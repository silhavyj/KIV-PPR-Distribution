#pragma once

#include <vector>
#include <cstdint>
#include <cstddef>
#include <iostream>
#include <functional>

namespace kiv_ppr
{
    class CHistogram
    {
    public:
        struct TParams
        {
            double min;
            double max;
            size_t number_of_intervals;
        };

    public:
        CHistogram(TParams params);
        ~CHistogram() = default;

        void Add(double value);

        [[nodiscard]] size_t Get_Number_Of_Intervals() const noexcept;
        [[nodiscard]] double Get_Min() const noexcept;
        [[nodiscard]] double Get_Max() const noexcept;
        [[nodiscard]] double Get_Interval_Size() const noexcept;
        [[nodiscard]] size_t Get_Total_Count() const;

        [[nodiscard]] size_t& operator[](uint32_t index);
        void operator+=(CHistogram& other);

        friend std::ostream& operator<<(std::ostream& out, CHistogram& histogram);

    private:
        std::vector<size_t> m_intervals;
        double m_interval_size;
        TParams m_params;
        size_t m_count;
    };
}