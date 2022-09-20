#include <iostream>

#include "Utils.h"
#include "FileReader.h"
#include "BasicFileStats.h"
#include "Histogram.h"
#include "Config.h"
#include "AdvancedFileStats.h"

int main()
{
    std::string filename{"data.dat"};

   // kiv_ppr::utils::Generate_Numbers<std::normal_distribution<>>(filename.c_str(), 10, 100, 20);

    kiv_ppr::File_Reader<double> file(filename);
    if (file.Is_Open())
    {
        std::cout << file << "\n";

        file.Calculate_Valid_Numbers(&kiv_ppr::utils::Double_Valid_Function, kiv_ppr::config::NUMBER_OF_THREADS_TO_READ_FILE, kiv_ppr::config::NUMBER_OF_ELEMENTS_PER_READ);
        kiv_ppr::Basic_File_Stats<double, double> basic_stats(&file, &kiv_ppr::utils::Double_Valid_Function);

        if (0 == basic_stats.Process(kiv_ppr::config::NUMBER_OF_THREADS_TO_READ_FILE))
        {
            const auto [min, max, mean] = basic_stats.Get_Values();

            std::cout << "min = " << min << "\n";
            std::cout << "max = " << max << "\n";
            std::cout << "mean = " << mean << "\n";

            kiv_ppr::Histogram<double>::Config histogram_config = { kiv_ppr::config::DEFAULT_NUMBER_OF_SLOTS, min, max };

            kiv_ppr::Advanced_File_Stats<double, double> advanced_stats(&file, &kiv_ppr::utils::Double_Valid_Function, basic_stats.Get_Values(), histogram_config);
            if (0 == advanced_stats.Process(kiv_ppr::config::NUMBER_OF_THREADS_TO_READ_FILE))
            {
                const auto [sd, histogram2] = advanced_stats.Get_Values();

                std::cout << "standard deviation = " << sd << "\n";
                std::cout << "histogram: " << *histogram2 << "\n";
            }
            else
            {
                std::cerr << "Error while processing the file (2)\n";
            }
        }
        else
        {
            std::cerr << "Error while processing the file (1)\n";
        }
    }
    else
    {
        std::cerr << "File not open\n";
    }
}