#pragma once

#include "Histogram.h"
#include "FileReader.h"
#include "BasicFileStats.h"

namespace kiv_ppr
{
    template<class T, class E>
    class Advanced_File_Stats
    {
    public:
        Advanced_File_Stats(File_Reader<E>* file, typename Basic_File_Stats<T, E>::Values values, uint32_t number_of_threads);
        ~Advanced_File_Stats() = default;

        [[nodiscard]] std::shared_ptr<Histogram<T>> Get_Histogram() noexcept;

        [[nodiscard]] int Process();

    private:
        void Report_Results(Histogram<T>& histogram) noexcept;
        [[nodiscard]] int Worker() noexcept;

    private:
        File_Reader<E>* m_file;
        typename Basic_File_Stats<T, E>::Values m_values;
        uint32_t m_number_of_threads;
        std::shared_ptr<Histogram<T>> m_histogram;
        std::mutex m_mtx;
    };
}