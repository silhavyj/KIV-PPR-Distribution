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

    static TThread_Params default_thread_params {
        std::thread::hardware_concurrency(),
        1024 * 1024 * 10,
        10
    };

    static constexpr uint32_t DOUBLE_PRECISION = 20;
}