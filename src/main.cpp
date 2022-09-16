#include <iostream>

#include <spdlog/spdlog.h>

#include "FileReader.h"
#include "FileStats.h"
#include "Histogram.h"

int main()
{
    //spdlog::set_level(spdlog::level::debug); // Set global log level to debug

    kiv_ppr::File_Reader<double> file("../../data.dat", 100 / sizeof(double));
    if (file.Is_Open())
    {
        kiv_ppr::File_Stats<double, double> stats(&file);
        if (0 == stats.Process(5))
        {
            fmt::print("min = {}\n", stats.Get_Min());
            fmt::print("max = {}\n", stats.Get_Max());
            fmt::print("mean = {}\n", stats.Get_Mean());

            auto histogram = stats.Get_Histogram(10);
            for (uint32_t i = 0; i < histogram.Get_Size(); ++i)
            {
                std::cout << histogram[i] << " ";
            }
        }
        else
        {
            spdlog::error("File not open");
        }
    }
    else
    {
        spdlog::error("File not open");
    }

    return 0;
}
