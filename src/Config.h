#pragma once

#include <cstdint>
#include <thread>

namespace kiv_ppr::config
{
    struct TThread_Params
    {
        uint32_t number_of_threads;
        uint32_t number_of_elements_per_file_read;
        double watchdog_expiration_sec;
    };

    static constexpr uint32_t DOUBLE_PRECISION = 5;
    static constexpr uint32_t BLOCK_SIZE_PER_READ = (1024 * 1024 * 10) / sizeof(double);
    static constexpr uint32_t WATCHDOG_EXPIRATION_SEC = 10;
    static constexpr double ALPHA_CRITICAL = 0.05;

    static TThread_Params default_thread_params {
        std::thread::hardware_concurrency(),
        BLOCK_SIZE_PER_READ,
        WATCHDOG_EXPIRATION_SEC
    };
}