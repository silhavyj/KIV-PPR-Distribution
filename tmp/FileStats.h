#pragma once

#include "FileReader.h"
#include "Histogram.h"

namespace kiv_ppr
{
    template<class T, class E>
    class File_Stats
    {
    public:
        File_Stats(File_Reader<E>* file);
        ~File_Stats() = default;

        [[nodiscard]] int Process(uint32_t thread_count);
        [[nodiscard]] Histogram<E> Get_Histogram(uint32_t number_of_slots);

        T Get_Min() const noexcept;
        T Get_Max() const noexcept;
        T Get_Mean() const noexcept;

    private:
        void Report(T min, T max, T mean) noexcept;
        void Update(T& min, T& max, T& mean, E value) const noexcept;
        [[nodiscard]] int Worker() noexcept;

        void Report_2(auto& values, Histogram<E>* histogram) noexcept;
        void Worker_2(Histogram<E>* histogram) noexcept;

    private:
        File_Reader<E>* m_file;
        T m_min;
        T m_max;
        T m_mean;
        std::mutex m_mtx;
    };
}