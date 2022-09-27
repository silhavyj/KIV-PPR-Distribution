#include <iostream>
#include <thread>
#include <cmath>
#include <cassert>
#include <iomanip>

#include "Utils.h"
#include "Config.h"
#include "FileReader.h"
#include "BasicFileStats.h"
#include "Histogram.h"
#include "AdvancedFileStats.h"
#include "NormalDistributionTest.h"
#include "WatchDog.h"

int main()
{
    /*kiv_ppr::CWatch_Dog watch_dog(5);

    std::thread t1([&watch_dog]() {
        assert(true == watch_dog.Register());
        assert(true == watch_dog.Kick());
        std::this_thread::sleep_for(std::chrono::seconds(6));
        assert(true == watch_dog.Unregister());
        std::cout << "Terminating thread1\n";
    });

    t1.join();*/

    std::string filename{"data.dat"};

    unsigned int cpu_cores = std::thread::hardware_concurrency();
    if (cpu_cores > 2)
    {
        cpu_cores -= 2; // Main thread + WatchDog
    }

    kiv_ppr::config::TThread_Config thread_config = {
        cpu_cores,
        1024 * 1024 * 10,
        10
    };

    std::cout << "Generating...\n";
    kiv_ppr::utils::Generate_Numbers<std::exponential_distribution<>>(filename.c_str(), 1000, 5);

    std::cout << kiv_ppr::utils::Time_Call([&filename, &thread_config]() {
        kiv_ppr::CFile_Reader<double> file(filename);

        std::cout << "Processing " << file.Get_Filename() << " (" << file.Get_File_Size() << " [B])\n";
        if (file.Is_Open())
        {
            // std::cout << file << "\n";

            file.Calculate_Valid_Numbers(&kiv_ppr::utils::Double_Valid_Function, thread_config);
            kiv_ppr::CBasic_File_Stats basic_stats(&file, &kiv_ppr::utils::Double_Valid_Function);

            if (0 == basic_stats.Process(thread_config))
            {
                const auto [min, max, mean] = basic_stats.Get_Values();

                std::cout << "min = " << std::setprecision(9) << min << "\n";
                std::cout << "max = " << std::setprecision(9) << max << "\n";
                std::cout << "mean = " << std::setprecision(9) << mean << "\n";

                kiv_ppr::CHistogram::TConfig histogram_config = {kiv_ppr::utils::Get_Number_Of_Intervals(file.Get_Total_Number_Of_Valid_Elements()), min, max };

                kiv_ppr::CAdvanced_File_Stats advanced_stats(&file, &kiv_ppr::utils::Double_Valid_Function, basic_stats.Get_Values(), histogram_config);
                if (0 == advanced_stats.Process(thread_config))
                {
                    const auto [sd, histogram] = advanced_stats.Get_Values();

                    std::cout << "standard deviation = " << std::setprecision(9) << sd << "\n";
                    std::cout << "histogram: " << *histogram << "\n";
                    auto histogram_mean = histogram->Get_Mean();
                    std::cout << "histogram mean: " << histogram->Get_Mean() << '\n';
                    std::cout << "histogram standard deviation: " << histogram->Get_Standard_Deviation(histogram_mean) << '\n';
                    std::cout << "Uniform distribution: " << kiv_ppr::utils::Is_Uniform_Distribution(histogram) << "\n";

                    kiv_ppr::CNormal_Distribution_Test normal_test(&file, &kiv_ppr::utils::Double_Valid_Function, mean, sd);
                    if (0 == normal_test.Process(thread_config))
                    {
                        std::cout << "Normal distribution = " << normal_test.Is_Normal_Distribution(0.1) << "\n";
                        std::cout << "68 -> " << normal_test.Get_Category_68() * 100 << '\n';
                        std::cout << "95 -> " << normal_test.Get_Category_95() * 100 << '\n';
                        std::cout << "99.5 -> " << normal_test.Get_Category_99_7() * 100 << '\n';
                    }
                    else
                    {
                        std::cerr << "Error while performing a normal distribution test\n";
                    }

                    histogram->Merge_Sparse_Intervals(5);
                    std::cout << "merged histogram: " << *histogram << "\n";
                    histogram_mean = histogram->Get_Mean();
                    std::cout << "histogram mean: " << histogram->Get_Mean() << '\n';
                    std::cout << "histogram standard deviation: " << histogram->Get_Standard_Deviation(histogram_mean) << '\n';
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