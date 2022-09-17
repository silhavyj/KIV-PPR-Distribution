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

    // spdlog::debug("Generating file...");
    // kiv_ppr::utils::Generate_Normal_Distribution_Test_File(filename, 80, 20, 2);

    spdlog::debug("Processing...");
    kiv_ppr::File_Reader<double> file(filename, 3);
    if (!file.Is_Open())
    {
        spdlog::error("Cannot open the input file");
        return 1;
    }

    kiv_ppr::Basic_File_Stats<double, double> basic_stats(&file, 1);
    if (0 != basic_stats.Process())
    {
        spdlog::error("Failed to process basic statistical information about the file");
        return 1;
    }

    fmt::print("min = {}\n", basic_stats.Get_Min());
    fmt::print("max = {}\n", basic_stats.Get_Max());
    fmt::print("mean = {}\n", basic_stats.Get_Mean());
    fmt::print("median = {}\n", basic_stats.Get_Median());

    std::cout << file << '\n';
}