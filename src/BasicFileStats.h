#pragma once

#include "MedianFinder.h"
#include "FileReader.h"

namespace kiv_ppr
{
    template<class T, class E>
    class Basic_File_Stats
    {
    public:
        struct Values
        {
            T min;
            T max;
            T mean;
            T median;
        };

    public:
        Basic_File_Stats(File_Reader<E>* file, uint32_t number_of_threads);
        ~Basic_File_Stats() = default;

        [[nodiscard]] T Get_Min() const noexcept;
        [[nodiscard]] T Get_Max() const noexcept;
        [[nodiscard]] T Get_Mean() const noexcept;
        [[nodiscard]] T Get_Median() const noexcept;
        [[nodiscard]] Values Get_Values() const noexcept;

        [[nodiscard]] int Process();

    private:
        void Report_Results(T min, T max, T mean, Stream_Median_Finder<T>& median_finder) noexcept;
        [[nodiscard]] int Worker() noexcept;

    private:
        File_Reader<E>* m_file;
        uint32_t m_number_of_threads;
        T m_min;
        T m_max;
        T m_mean;
        T m_median;
        Stream_Median_Finder<T> m_median_finder;
        std::mutex m_mtx;
    };
}