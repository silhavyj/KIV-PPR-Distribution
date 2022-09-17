#include <iostream>
#include <limits>
#include <cmath>

#include <spdlog/spdlog.h>

#include "Utils.h"
#include "FileReader.h"
#include "BasicFileStats.h"
#include "HistogramBuilder.h"

int main()
{
    spdlog::set_level(spdlog::level::debug);
    // spdlog::set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");

    std::string filename{"data.dat"};
    kiv_ppr::utils::Generate_Normal_Distribution_Test_File(filename, 10 * 1024, 5, 2);

    kiv_ppr::File_Reader<double> file(filename, 100);
    if (!file.Is_Open())
    {
        spdlog::error("Cannot open the input file");
        return 1;
    }
    std::cout << file << "\n";

    kiv_ppr::Basic_File_Stats<double, double> basic_stats(&file, 3);
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
}