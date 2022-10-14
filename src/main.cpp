#include <iostream>

#include "cxxopts/cxxopts.hpp"

#include "utils/Utils.h"
#include "Config.h"
#include "FileReader.h"
#include "processing/FileStats.h"
#include "tests/TestRunner.h"

// static std::string filename{"../../data/poisson"};
// static std::string filename{"/home/silhavyj/Downloads/poissonlarger"};
// static std::string filename{ "data2.dat" };
// static std::string filename{ "data.dat" };
// static std::string filename{"/home/silhavyj/Downloads/ubuntu-22.04.1-desktop-amd64.iso"};
// static std::string filename{"/home/silhavyj/Downloads/a"};

static void Run(const char* filename, double p_critical)
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

        kiv_ppr::CTest_Runner test_runner(values, p_critical);
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
    cxxopts::Options options("./pprsolver <filename> <all | SMP | dev1 dev2 dev3 ...>", "KIV/PPR Semester project - Classification of statistical distributions");
    options.add_options()
            ("p,p_critical", "Critical p value used in the chi square test", cxxopts::value<double>()->default_value("0.05"))
            ("h,help", "Print out this help menu");

    cxxopts::ParseResult args = options.parse(argc, argv);
    if (args.count("help"))
    {
        std::cout << options.help() << std::endl;
        return 0;
    }
    if (argc < 3)
    {
        std::cerr << "Invalid number of input parameters\n";
        std::cerr << "Run './pprsolver --help'\n";
        return 1;
    }
    double p_critical = args["p_critical"].as<double>();
    if (p_critical < 0)
    {
        std::cerr << "p_critical (" << p_critical << ") must be a positive number\n";
        return 1;
    }

    const auto seconds = kiv_ppr::utils::Time_Call([&argv, &p_critical]() {
        Run(argv[1], p_critical);
    });
    std::cout << "\nTime of execution: " << seconds << " sec\n";
}