#pragma once

namespace kiv_ppr::config
{
    static constexpr auto TEST_FILE_SIZE = (3L * 1024 * 1024 * 1024);     // 3GB
    static constexpr auto FILE_BLOCK_SIZE = (1L * 1024) / sizeof(double); // 1KB
    static constexpr auto NUMBER_OF_WORKERS = 5;
}