#include <iostream>
#include <limits>
#include <cmath>

#include <spdlog/spdlog.h>

#include "Utils.h"
#include "FileReader.h"
#include "BasicFileStats.h"
#include "HistogramBuilder.h"
#include "AdvancedFileStats.h"
#include "Config.h"

int main()
{
    spdlog::set_level(spdlog::level::debug);
    // spdlog::set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");

    std::string filename{"data.dat"};

    /*spdlog::debug("Generating file...");
    kiv_ppr::utils::Generate_Normal_Distribution_Test_File(filename, kiv_ppr::config::TEST_FILE_SIZE, 100, 20);*/

    spdlog::debug("Processing...");
    kiv_ppr::File_Reader<double> file(filename, kiv_ppr::config::FILE_BLOCK_SIZE);
    if (!file.Is_Open())
    {
        spdlog::error("Cannot open the input file");
        return 1;
    }
    // std::cout << file << "\n";

    kiv_ppr::Basic_File_Stats<double, double> basic_stats(&file, kiv_ppr::config::NUMBER_OF_WORKERS);
    if (0 != basic_stats.Process())
    {
        spdlog::error("Failed to process basic statistical information about the file");
        return 1;
    }

    std::cout << "min = " << basic_stats.Get_Min() << "\n";
    std::cout << "max = " << basic_stats.Get_Max() << "\n";
    std::cout << "mean = " << basic_stats.Get_Mean() << "\n";

    auto histogram = kiv_ppr::histogram::Build_Histogram(&file, 10, basic_stats.Get_Min(), basic_stats.Get_Max());
    std::cout << histogram << "\n";

    auto values = basic_stats.Get_Values();
    kiv_ppr::Advanced_File_Stats<double, double> advanced_stats(&file, values, kiv_ppr::config::NUMBER_OF_WORKERS);
    if (0 != advanced_stats.Process())
    {
        spdlog::error("Failed to process advanced statistical information about the file");
        return 1;
    }

    auto his = advanced_stats.Get_Histogram();
    std::cout << *his.get() << "\n";
}