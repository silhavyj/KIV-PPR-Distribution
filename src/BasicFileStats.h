#pragma once

#include "Config.h"
#include "FileReader.h"
#include "WatchDog.h"

namespace kiv_ppr
{
    template<class T, class E>
    class CBasic_File_Stats
    {
    public:
        struct TValues
        {
            T min;
            T max;
            T mean;
        };

    public:
        explicit CBasic_File_Stats(CFile_Reader<E>* file, std::function<bool(E)> num_valid_fce);
        ~CBasic_File_Stats() = default;

        [[nodiscard]] T Get_Min() const noexcept;
        [[nodiscard]] T Get_Max() const noexcept;
        [[nodiscard]] T Get_Mean() const noexcept;
        [[nodiscard]] TValues Get_Values() const noexcept;

        [[nodiscard]] int Process(config::TThread_Config thread_config);

    private:
        void Report_Results(T min, T max, T mean) noexcept;
        [[nodiscard]] int Worker(const config::TThread_Config& thread_config, CWatch_Dog* watch_dog);

    private:
        CFile_Reader<E>* m_file;
        T m_min;
        T m_max;
        std::function<bool(E)> m_num_valid_fce;
        T m_mean;
        std::mutex m_mtx;
    };
}