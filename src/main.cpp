#include <iostream>
#include <thread>

#include "utils/Utils.h"
#include "Config.h"
#include "FileReader.h"
#include "processing/FileStats.h"
#include "tests/TestRunner.h"

static std::string filename{"../../data/exp"};
// static std::string filename{"/home/silhavyj/Downloads/poissonlarger"};
// static std::string filename{ "data2.dat" };
// static std::string filename{ "data.dat" };
// static std::string filename{"/home/silhavyj/Downloads/ubuntu-22.04.1-desktop-amd64.iso"};
// static std::string filename{"/tmp/a"};

static void Run()
{
    kiv_ppr::CFile_Reader<double> file(filename);
    if (file.Is_Open())
    {
        if (file.Get_File_Size() < sizeof(double))
        {
            std::cerr << "The size of the input file is insufficient\n";
            std::exit(1);
        }

        std::cout << "Processing file " << file.Get_Filename() << " [" << file.Get_File_Size() << " B]\n";

        kiv_ppr::CFile_Stats file_stats(&file, kiv_ppr::utils::Is_Valid_Double);
        if (0 != file_stats.Process(&kiv_ppr::config::default_thread_params))
        {
            std::cerr << "Failed to process the input file (" << file.Get_Filename() << ")\n";
            std::exit(1);
        }
        auto values = file_stats.Get_Values();

        std::cout << "Calculated statistics (parameters):\n";
        std::cout << values << "\n\n";

        // std::cout << *values.second_iteration.histogram << "\n";

        kiv_ppr::CTest_Runner test_runner(values);
        test_runner.Run();
    }
    else
    {
        std::cerr << "Failed to open the input file (" << file.Get_Filename() << ")\n";
        std::exit(1);
    }
}

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    // kiv_ppr::utils::Generate_Numbers<std::poisson_distribution<>>(filename.c_str(), false, 1000, 2);

    const auto seconds = kiv_ppr::utils::Time_Call([]() {
        Run();
    });
    std::cout << "\nTime of execution: " << seconds << " sec\n";
}