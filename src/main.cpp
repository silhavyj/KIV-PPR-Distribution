#include <iostream>
#include <thread>

#include "utils/Utils.h"
#include "Config.h"
#include "FileReader.h"
#include "processing/FileStats.h"
#include "tests/TestRunner.h"
#include "cdfs/PoissonCDF.h"
#include "utils/Utils.h"

static std::string filename{"../../data/gauss"};
// static std::string filename{ "data.dat" };
// static std::string filename{"/home/silhavyj/Downloads/ubuntu-22.04.1-desktop-amd64.iso"};

static void Run()
{
    kiv_ppr::CFile_Reader<double> file(filename);
    if (file.Is_Open())
    {
        std::cout << "Processing file " << file.Get_Filename() << " [" << file.Get_File_Size() << " B]\n";

        kiv_ppr::CFile_Stats file_stats(&file, kiv_ppr::utils::Is_Valid_Double);
        if (0 != file_stats.Process(&kiv_ppr::config::default_thread_params))
        {
            std::cerr << "Failed to process the input file\n";
            std::exit(1);
        }
        auto values = file_stats.Get_Values();

        std::cout << "Calculated statistics:\n";
        std::cout << values << "\n\n";

        // std::cout << *values.second_iteration.histogram << "\n";

        kiv_ppr::CTest_Runner test_runner(values);
        test_runner.Run();
    }
    else
    {
        std::cerr << "Failed to open the input file\n";
        std::exit(1);
    }
}

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    // kiv_ppr::utils::Generate_Numbers<std::uniform_real_distribution<>>(filename.c_str(), true, 1000, std::numeric_limits<double>::max() / 2 , std::numeric_limits<double>::max() - 1);

    const auto seconds = kiv_ppr::utils::Time_Call([]() {
        Run();
    });
    std::cout << "\nTime of execution: " << seconds << " sec\n";


    /*std::cout << '\n';
    kiv_ppr::CPoisson_CDF p(100000);
    std::cout << p(100000) << '\n';

    int err;
    std::cout << kiv_ppr::utils::gammad(5, 1000, &err);
    std::cout << err << '\n';*/
}