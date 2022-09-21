#pragma once

#include <cstddef>
#include <string>
#include <fstream>
#include <random>
#include <chrono>
#include <cstdint>

namespace kiv_ppr::utils
{
    template <typename D, typename ...arg>
    void Generate_Numbers(const char* filename, const size_t count, const arg... a)
    {
        std::ofstream output(filename, std::ios::out | std::ios::binary);
        if (output.is_open())
        {
            std::random_device rd;
            D dis(a...);

            for (size_t n = 0; n < count; ++n)
            {
                const double num = dis(rd);
                output.write(reinterpret_cast<const char*>(&num), sizeof(num));
            }
        }
    }

    template<typename Function>
    uint32_t Time_Call(Function&& function)
    {
        const auto start_time = std::chrono::steady_clock::now();
        function();
        const auto end_time = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count();
    }

    bool Double_Valid_Function(double value);
}