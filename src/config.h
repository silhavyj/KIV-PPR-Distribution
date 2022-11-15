#pragma once

#include <cstdint>
#include <thread>

namespace kiv_ppr::config
{
    namespace chi_square
    {
        /// Minimum expected value (CDF(x) * count)
        static constexpr double Min_Expected_Value = 5;

        /// Default critical P-Value
        static constexpr double Default_P_Critical = 0.05;
    }

    namespace processing
    {
        /// Default block size (10 MB)
        static constexpr uint32_t Block_Size_Per_Read = 1024 * 1024 * 10;

        // Default Watchdog sleep period (5s)
        static constexpr uint32_t Watchdog_Sleep_Sec = 5;

        // Scale factor for all the input values
        static constexpr double Scale_Factor = 2.0;
    }
    
    // Precision used when printing out double values. 
    static constexpr uint32_t Double_Precision = 5;

    /// Thread configuration when processing the input file.
    struct TThread_Params
    {
        uint32_t number_of_threads;                ///< Number of threads used to process the file
        uint32_t number_of_elements_per_file_read; ///< Number of elements read from the file at once
        double watchdog_expiration_sec;            ///< Watchdog period
    };

    /// Default thread settings.
    static TThread_Params default_thread_params {
        std::thread::hardware_concurrency(), // Number of threads of the CPU
        processing::Block_Size_Per_Read,
        processing::Watchdog_Sleep_Sec
    };
}

// EOF