#include <iostream>

#include <spdlog/spdlog.h>

#include "FileReader.h"
#include "FileStats.h"
#include "Histogram.h"
#include "Utils.h"

int main()
{
    // spdlog::set_level(spdlog::level::debug); // Set global log level to debug

    std::string filename{"data.dat"};
    kiv_ppr::utils::Generate_Normal_Distribution_Test_File(filename, 80, 10, 2);

    std::ifstream file(filename, std::ios::binary);
    while (!file.eof())
    {
        auto buffer = std::shared_ptr<double[]>(new(std::nothrow) double[3]);
        file.read(reinterpret_cast<char*>(buffer.get()), 3 * sizeof(double));

        for (unsigned int i = 0; i < file.gcount() / sizeof(double); ++i)
        {
            std::cout << buffer[i] << " ";
        }
    }


    /*{
        kiv_ppr::File_Reader<double> file(filename, 8);
        if (file.Is_Open()) {
            kiv_ppr::File_Stats<double, double> stats(&file);
            if (0 == stats.Process(5)) {
                fmt::print("min = {}\n", stats.Get_Min());
                fmt::print("max = {}\n", stats.Get_Max());
                fmt::print("mean = {}\n", stats.Get_Mean());

                std::cout << file << '\n';

                auto histogram = stats.Get_Histogram(10);
                for (uint32_t i = 0; i < histogram.Get_Size(); ++i) {
                    std::cout << histogram[i] << " ";
                }
                std::cout << file << '\n';
            }
            else
            {
                spdlog::error("File not open");
            }
        }
        else {
            spdlog::error("File not open");
        }
    }*/

    return 0;
}
