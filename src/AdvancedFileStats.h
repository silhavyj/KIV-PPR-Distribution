#pragma once

#include <memory>

#include "Config.h"
#include "FileReader.h"
#include "Histogram.h"
#include "BasicFileStats.h"
#include "WatchDog.h"

namespace kiv_ppr
{
    class CAdvanced_File_Stats
    {
    public:
        struct TValues
        {
            double standard_deviation;
            std::shared_ptr<CHistogram> histogram;
        };

    public:
        explicit CAdvanced_File_Stats(CFile_Reader<double>* file,
                                      std::function<bool(double)> num_valid_fce,
                                      typename CBasic_File_Stats::TValues basic_values,
                                      typename CHistogram::TConfig histogram_config);

        ~CAdvanced_File_Stats() = default;

        [[nodiscard]] double Get_Standard_Deviation() const noexcept;
        [[nodiscard]] std::shared_ptr<CHistogram> Get_Histogram() const noexcept;
        [[nodiscard]] TValues Get_Values() const noexcept;

        [[nodiscard]] int Process(config::TThread_Config thread_config);

    private:
        void Report_Results(double standard_deviation, CHistogram& histogram) noexcept;
        [[nodiscard]] int Worker(const config::TThread_Config* thread_config, CWatch_Dog* watch_dog) noexcept;

    private:
        CFile_Reader<double>* m_file;
        std::function<bool(double)> m_num_valid_fce;
        typename CBasic_File_Stats::TValues m_basic_values;
        typename CHistogram::TConfig m_histogram_config;
        double m_standard_deviation;
        std::shared_ptr<CHistogram> m_histogram;
        std::mutex m_mtx;
    };
}