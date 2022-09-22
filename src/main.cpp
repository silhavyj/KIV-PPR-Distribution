#include <iostream>
#include <thread>
#include <cmath>

#include "Utils.h"
#include "Config.h"
#include "FileReader.h"
#include "BasicFileStats.h"
#include "Histogram.h"
#include "AdvancedFileStats.h"
#include "WatchDog.h"

int main()
{
    /*kiv_ppr::CWatch_Dog watch_dog(0.01, 2);

    std::thread t1([&watch_dog]() {
        for (int i = 0; i < 10; ++i)
        {
            watch_dog.Kick();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });

    std::thread t2([&watch_dog]() {
        for (int i = 0; i < 10; ++i)
        {
            watch_dog.Kick();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });

    t1.join();
    t2.join();*/

    std::string filename{"data.dat"};

    kiv_ppr::config::TThread_Config thread_config = {
        std::thread::hardware_concurrency(), // TODO minus WatchDog, Main thread
        1024 * 1024 * 10,
        10
    };

    // kiv_ppr::utils::Generate_Numbers<std::normal_distribution<>>(filename.c_str(), 134217728, 100, 20);

    std::cout << kiv_ppr::utils::Time_Call([&filename, &thread_config]() {
        kiv_ppr::CFile_Reader<double> file(filename);
        if (file.Is_Open())
        {
            // std::cout << file << "\n";

            file.Calculate_Valid_Numbers(&kiv_ppr::utils::Double_Valid_Function, thread_config);
            kiv_ppr::CBasic_File_Stats<double, double> basic_stats(&file, &kiv_ppr::utils::Double_Valid_Function);

            if (0 == basic_stats.Process(thread_config))
            {
                const auto [min, max, mean] = basic_stats.Get_Values();

                std::cout << "min = " << min << "\n";
                std::cout << "max = " << max << "\n";
                std::cout << "mean = " << mean << "\n";

                kiv_ppr::CHistogram<double>::TConfig histogram_config = {kiv_ppr::CHistogram<double>::DEFAULT_NUMBER_OF_SLOTS, min, max };

                kiv_ppr::CAdvanced_File_Stats<double, double> advanced_stats(&file, &kiv_ppr::utils::Double_Valid_Function, basic_stats.Get_Values(), histogram_config);
                if (0 == advanced_stats.Process(thread_config))
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
    }) << "s\n";
}