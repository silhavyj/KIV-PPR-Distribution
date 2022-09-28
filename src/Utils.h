#pragma once

#include <cstddef>
#include <string>
#include <fstream>
#include <random>
#include <chrono>
#include <cstdint>
#include <iostream>

#include "FileReader.h"

namespace kiv_ppr::utils
{
    template <typename Distribution, typename ...Args>
    void Generate_Numbers(const char* filename, bool verbose, const size_t count, const Args... args)
    {
        if (verbose)
        {
            std::cout << "Generating data...\n";
        }
        std::ofstream output(filename, std::ios::out | std::ios::binary);
        size_t ten_percent = static_cast<size_t>(0.1f * count);
        uint32_t percentage = 0;

        if (output.is_open())
        {
            std::random_device rd;
            Distribution dis(args...);

            for (size_t i = 0; i < count; ++i)
            {
                const double num = dis(rd);
                output.write(reinterpret_cast<const char *>(&num), sizeof(num));

                if (verbose && i % ten_percent == 0)
                {
                    std::cout << percentage << "%\n";
                    percentage += 10;
                }
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

    bool Is_Valid_Double(double value);

    double Calculate_Mean_Sequential(kiv_ppr::CFile_Reader<double>& file, uint32_t number_of_elements_per_file_read);
}