#pragma once

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
        };

    public:
        explicit Basic_File_Stats(File_Reader<E>* file, std::function<bool(E)> num_valid_fce);
        ~Basic_File_Stats() = default;

        [[nodiscard]] T Get_Min() const noexcept;
        [[nodiscard]] T Get_Max() const noexcept;
        [[nodiscard]] T Get_Mean() const noexcept;
        [[nodiscard]] Values Get_Values() const noexcept;

        [[nodiscard]] int Process(uint32_t number_of_threads);

    private:
        void Report_Results(T min, T max, T mean) noexcept;
        [[nodiscard]] int Worker() noexcept;

    private:
        File_Reader<E>* m_file;
        T m_min;
        T m_max;
        std::function<bool(E)> m_num_valid_fce;
        T m_mean;
        std::mutex m_mtx;
    };
}