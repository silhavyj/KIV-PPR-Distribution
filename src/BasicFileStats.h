#pragma once

#include <vector>
#include <future>

#include "Config.h"
#include "FileReader.h"
#include "WatchDog.h"

namespace kiv_ppr
{
    class CBasic_File_Stats
    {
    public:
        struct TValues
        {
            double min;
            double max;
            double mean;
        };

    public:
        explicit CBasic_File_Stats(CFile_Reader<double>* file, std::function<bool(double)> num_valid_fce);
        ~CBasic_File_Stats() = default;

        [[nodiscard]] double Get_Min() const noexcept;
        [[nodiscard]] double Get_Max() const noexcept;
        [[nodiscard]] double Get_Mean() const noexcept;
        [[nodiscard]] TValues Get_Values() const noexcept;

        [[nodiscard]] int Process(config::TThread_Config thread_config);

    private:
        void Report_Results(double min, double max, double mean) noexcept;
        [[nodiscard]] int Worker(const config::TThread_Config* thread_config, CWatch_Dog* watch_dog);

    private:
        CFile_Reader<double>* m_file;
        std::function<bool(double)> m_num_valid_fce;
        double m_min;
        double m_max;
        double m_mean;
        std::mutex m_mtx;
    };
}