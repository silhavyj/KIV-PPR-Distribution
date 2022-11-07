#pragma once

#include <cstdint>
#include <thread>

namespace kiv_ppr::config
{
    namespace chi_square
    {
        static constexpr double Min_Expected_Value = 5;
        static constexpr double Default_P_Critical = 0.05;
    }

    namespace processing
    {
        static constexpr uint32_t Block_Size_Per_Read = (1024 * 1024 * 10) / sizeof(double);
        static constexpr uint32_t Watchdog_Sleep_Sec = 3;
        static constexpr double Scale_Factor = 2.0;
    }
    
    static constexpr uint32_t Double_Precision = 5;

    struct TThread_Params
    {
        uint32_t number_of_threads;
        uint32_t number_of_elements_per_file_read;
        double watchdog_expiration_sec;
    };

    static TThread_Params default_thread_params {
        std::thread::hardware_concurrency(),
        processing::Block_Size_Per_Read,
        processing::Watchdog_Sleep_Sec
    };
}