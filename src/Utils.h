#pragma once

#include <cstddef>
#include <string>

namespace kiv_ppr::utils
{
    int Generate_Uniform_Distribution_Test_File(const std::string& filename, std::size_t size, double min, double max);
    int Generate_Normal_Distribution_Test_File(const std::string& filename, std::size_t size, double mean, double SD);
}