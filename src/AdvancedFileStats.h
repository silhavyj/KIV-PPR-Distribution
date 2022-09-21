#pragma once

#include <memory>

#include "Config.h"
#include "FileReader.h"
#include "Histogram.h"
#include "BasicFileStats.h"

namespace kiv_ppr
{
    template<class T, class E>
    class CAdvanced_File_Stats
    {
    public:
        struct TValues
        {
            T standard_deviation;
            std::shared_ptr<CHistogram<T>> histogram;
        };

    public:
        explicit CAdvanced_File_Stats(CFile_Reader<E>* file,
                                      std::function<bool(E)> num_valid_fce,
                                      typename CBasic_File_Stats<T, E>::TValues basic_values,
                                      typename CHistogram<T>::TConfig histogram_config);

        ~CAdvanced_File_Stats() = default;

        [[nodiscard]] T Get_Standard_Deviation() const noexcept;
        [[nodiscard]] std::shared_ptr<CHistogram<T>> Get_Histogram() const noexcept;
        [[nodiscard]] TValues Get_Values() const noexcept;

        [[nodiscard]] int Process(config::TThread_Config thread_config);

    private:
        void Report_Results(T standard_deviation, CHistogram<T>& histogram) noexcept;
        [[nodiscard]] int Worker(const config::TThread_Config& thread_config) noexcept;

    private:
        CFile_Reader<T>* m_file;
        std::function<bool(T)> m_num_valid_fce;
        typename CBasic_File_Stats<T, E>::TValues m_basic_values;
        typename CHistogram<T>::TConfig m_histogram_config;
        T m_standard_deviation;
        std::shared_ptr<CHistogram<T>> m_histogram;
        std::mutex m_mtx;
    };
}