#include <iostream>
#include <limits>
#include <cmath>
#include <chrono>

#include <spdlog/fmt/fmt.h>
#include <spdlog/spdlog.h>

#include "Utils.h"
#include "FileReader.h"
#include "BasicFileStats.h"

int main()
{
    spdlog::set_level(spdlog::level::debug);
    // spdlog::set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");

    std::string filename{"data.dat"};

    spdlog::debug("Generating file...");
    // kiv_ppr::utils::Generate_Normal_Distribution_Test_File(filename, 1024, 20, 2);
    // kiv_ppr::utils::Generate_Uniform_Distribution_Test_File(filename, 80, std::numeric_limits<double>::max() / 2, std::numeric_limits<double>::max());

    auto start_time = std::chrono::steady_clock::now();

    spdlog::debug("Processing...");
    kiv_ppr::File_Reader<double> file(filename, 1024 * 1024);
    if (!file.Is_Open())
    {
        spdlog::error("Cannot open the input file");
        return 1;
    }

    kiv_ppr::Basic_File_Stats<double, double> basic_stats(&file, 10);
    if (0 != basic_stats.Process())
    {
        spdlog::error("Failed to process basic statistical information about the file");
        return 1;
    }

    const auto [min, max, mean, median] = basic_stats.Get_Values();

    fmt::print("min = {}\n", min);
    fmt::print("max = {}\n", max);
    fmt::print("mean = {}\n", mean);
    fmt::print("median = {}\n", median);

    auto end_time = std::chrono::steady_clock::now();

    std::cout << file << '\n';
    // std::cout << "done\n";

    std::cout << std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count() << "s\n";
}