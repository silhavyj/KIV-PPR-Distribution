#pragma once

#include "FileReader.h"

namespace kiv_ppr
{
    template<class T, class E>
    class Basic_File_Stats
    {
    public:
        Basic_File_Stats(File_Reader<E>* file, uint32_t number_of_threads);
        ~Basic_File_Stats() = default;

        T Get_Min() const noexcept;
        T Get_Max() const noexcept;
        T Get_Mean() const noexcept;

        [[nodiscard]] int Process();

    private:
        void Report_Results(T min, T max, T mean) noexcept;
        void Update_Values(T& min, T& max, T& mean, E value) const noexcept;
        [[nodiscard]] int Worker() noexcept;

    private:
        File_Reader<E>* m_file;
        uint32_t m_number_of_threads;
        T m_min;
        T m_max;
        T m_mean;
        std::mutex m_mtx;
    };
}