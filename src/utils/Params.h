#pragma once

#include <cstdint>

namespace kiv_ppr::params
{
    struct TThread_Params
    {
        uint32_t number_of_threads;
        uint32_t number_of_elements_per_file_read;
        double watchdog_expiration_sec;
    };
}