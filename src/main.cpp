#include <iostream>
#include <functional>

#include "Utils.h"
#include "FileReader.h"
#include "BasicFileStats.h"
#include "Histogram.h"
#include "Config.h"

int main()
{
    std::string filename{"data.dat"};

    /* std::cout << "Generating file...\n";
    kiv_ppr::utils::Generate_Numbers<std::normal_distribution<>>(filename.c_str(), 268435456, 100, 20);
    std::cout << "Done"; */

    kiv_ppr::File_Reader<double> file(filename);
    if (file.Is_Open())
    {
        file.Calculate_Valid_Numbers(&kiv_ppr::utils::Double_Valid_Function, 8, 1024);
        std::cout << file.Get_Total_Number_Of_Elements() << "\n";
        std::cout << file.Get_Total_Number_Of_Valid_Elements() << "\n";
    }

    /*kiv_ppr::File_Reader<double> file(filename);
    if (file.Is_Open())
    {
        // Foo(file);
        // std::cout << file << "\n";

        kiv_ppr::Basic_File_Stats<double, double> basic_stats(&file);

        if (0 == basic_stats.Process(kiv_ppr::config::NUMBER_OF_THREADS_TO_READ_FILE))
        {
            const auto [min, max, mean] = basic_stats.Get_Values();

            std::cout << "min = " << min << "\n";
            std::cout << "max = " << max << "\n";
            std::cout << "mean = " << mean << "\n";

            auto histogram = kiv_ppr::Histogram<double>::Generate_Histogram(file, {kiv_ppr::config::DEFAULT_NUMBER_OF_SLOTS, min, max}, kiv_ppr::config::NUMBER_OF_THREADS_TO_READ_FILE);
            std::cout << histogram << "\n";
        }
        else
        {
            std::cerr << "Error while processing the file\n";
        }
    }
    else
    {
        std::cerr << "File not open\n";
    }*/
}