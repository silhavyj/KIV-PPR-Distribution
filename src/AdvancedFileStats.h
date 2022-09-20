#pragma once

#include <memory>

#include "FileReader.h"
#include "Histogram.h"
#include "BasicFileStats.h"

namespace kiv_ppr
{
    template<class T, class E>
    class Advanced_File_Stats
    {
    public:
        struct Values
        {
            T standard_deviation;
            std::shared_ptr<Histogram<T>> histogram;
        };

    public:
        explicit Advanced_File_Stats(File_Reader<E>* file, std::function<bool(E)> num_valid_fce, typename Basic_File_Stats<T, E>::Values basic_values, typename Histogram<T>::Config histogram_config);
        ~Advanced_File_Stats() = default;

        [[nodiscard]] T Get_Standard_Deviation() const noexcept;
        [[nodiscard]] std::shared_ptr<Histogram<T>> Get_Histogram() const noexcept;
        [[nodiscard]] Values Get_Values() const noexcept;

        [[nodiscard]] int Process(uint32_t number_of_threads);

    private:
        void Report_Results(T standard_deviation, Histogram<T>& histogram) noexcept;
        [[nodiscard]] int Worker() noexcept;

    private:
        File_Reader<T>* m_file;
        std::function<bool(T)> m_num_valid_fce;
        typename Basic_File_Stats<T, E>::Values m_basic_values;
        typename Histogram<T>::Config m_histogram_config;
        T m_standard_deviation;
        std::shared_ptr<Histogram<T>> m_histogram;
        std::mutex m_mtx;
    };
}