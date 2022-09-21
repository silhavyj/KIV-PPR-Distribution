#pragma once

#include <cstdint>

namespace kiv_ppr::config
{
    struct TThread_Config
    {
        uint32_t number_of_threads;
        uint32_t number_of_elements_per_file_read;
    };
}