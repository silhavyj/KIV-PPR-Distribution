#pragma once

#include <cstdint>

namespace kiv_ppr::config
{
    static constexpr uint32_t NUMBER_OF_THREADS_TO_READ_FILE = 2;
    static constexpr std::size_t NUMBER_OF_ELEMENTS_PER_READ = 2;
    static constexpr uint32_t DEFAULT_NUMBER_OF_SLOTS = 10;
}