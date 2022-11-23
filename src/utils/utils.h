#pragma once

#include <cstddef>
#include <string>
#include <array>
#include <fstream>
#include <random>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <functional>
#include <immintrin.h>

namespace kiv_ppr::utils
{
    /// Generates random numbers and stores them into a binary file.
    /// This method is a helper method used when testing the program.
    /// 
    /// \tparam Distribution Type of distribution (normal, exponential, ...)
    /// \tparam ...Args Arguments of the distribution (mean, lambda, ...)
    ///
    /// \param filename Name of the output file
    /// \param verbose Print out progress (%)
    /// \param count Number of values to be generated
    /// \param ...args Arguments of the distribution (mean, lambda, ...)
    template <typename Distribution, typename ...Args>
    void Generate_Numbers(const char* filename, bool verbose, const size_t count, const Args... args)
    {
        if (verbose)
        {
            std::cout << "Generating data...\n";
        }

        std::ofstream output(filename, std::ios::out | std::ios::binary);
        auto ten_percent = static_cast<size_t>(0.1f * static_cast<double>(count));

        if (output.is_open())
        {
            uint32_t percentage = 0;
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

    /// Measures how much time it takes to execute a function given as a parameter.
    /// \tparam Function Type of the function
    /// \param function Function to be called
    /// \return Number of seconds it took to call the function
    template<typename Function>
    auto Time_Call(Function&& function)
    {
        const auto start_time = std::chrono::steady_clock::now();
        function();
        const auto end_time = std::chrono::steady_clock::now();

        // Calculate how many seconds it took.
        return std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count();
    }

    /// Check if a value is a valid double or not.
    /// \param value Value to be tested
    /// \return true, if the value is a valid double, false otherwise.
    bool Is_Valid_Double(double value) noexcept;

    namespace vectorization
    {
        /// Aggregates results calculated using SIMD instructions.
        /// \vals Resluts (final values)
        /// \default_value Default value of the final aggregation
        /// \fce Function used to aggregate the values
        double Aggregate(const __m256d& vals, double default_value, std::function<double(double, double)> fce);

        /// Creates an __m256d (four doubles) - used for SIMD instructions.
        /// \data Data the __m256d will be created out of
        /// \offset Offset within the array (indexes higher than the offset are not used)
        /// \value Default value for the unused indexes
        __m256d Create_4Doubles(std::array<double, 4>& data, const std::size_t offset, double value);
    }
}

// EOF