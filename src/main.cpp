#include <iostream>
#include <limits>
#include <cmath>

#include <spdlog/spdlog.h>

#include "Utils.h"
#include "FileReader.h"
#include "Histogram.h"

int main()
{
    spdlog::set_level(spdlog::level::debug);

    std::string filename{"data.dat"};
    kiv_ppr::utils::Generate_Normal_Distribution_Test_File(filename, 1024, 5, 2);

    /*kiv_ppr::File_Reader<double> file(filename, 1024 / 8);
    if (!file.Is_Open())
    {
        return 1;
    }
    // std::cout << file << '\n';

    file.Seek_Beg();
    const auto [status, count, data] = file.Read_Data();
    double min = std::numeric_limits<double>::max();
    double max = std::numeric_limits<double>::min();

    if (status == kiv_ppr::File_Reader<double>::Status::OK)
    {
        for (std::size_t i = 0; i < count; ++i)
        {
            //std::cout << data[i] << " ";
            max = std::max(max, data[i]);
            min = std::min(min, data[i]);
        }
        std::cout << "\n";
    }

    kiv_ppr::Histogram<double> histogram(10, min, max);

    file.Seek_Beg();
    const auto block = file.Read_Data();

    if (block.status == kiv_ppr::File_Reader<double>::Status::OK)
    {
        for (std::size_t i = 0; i < count; ++i)
        {
            //std::cout << data[i] << " ";
            histogram.Add(data[i]);
        }
        std::cout << "\n";
    }
    std::cout << histogram << "\n";*/
}