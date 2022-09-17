#pragma once

#include <queue>
#include <vector>

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
        class Stream_Median_Finder
        {
        public:
            void Add_Value(T value);
            T Get_Median() const;
            bool Is_Empty() const;
            T Pop_Value();

        private:
            std::priority_queue<T> m_left_half;
            std::priority_queue<T, std::vector<T>, std::greater<T>> m_right_half;
        };

    private:
        void Report_Results(T min, T max, T mean, Stream_Median_Finder& median_finder) noexcept;
        [[nodiscard]] int Worker() noexcept;

    private:
        File_Reader<E>* m_file;
        uint32_t m_number_of_threads;
        T m_min;
        T m_max;
        T m_mean;
        T m_median;
        Stream_Median_Finder m_median_finder;
        std::mutex m_mtx;
    };
}