#include <iostream>

#include "utils/Utils.h"
#include "utils/ArgParser.h"
#include "utils/ResourceManager.h"
#include "utils/FileReader.h"
#include "utils/Singleton.h"
#include "Config.h"
#include "processing/FileStats.h"
#include "tests/TestRunner.h"

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

        std::cout << "\nCalculated statistics (parameters):\n";
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
    kiv_ppr::CArg_Parser arg_parser(argc, argv);
    try
    {
        arg_parser.Parse_Options();
        if (arg_parser.Help())
        {
            arg_parser.Print_Help();
            return 0;
        }
        arg_parser.Parse();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error occurred when parsing input parameters - " << e.what();
        std::cerr << "\nRun './pprsolver --help'\n";
        return 1;
    }

    double p_critical = arg_parser.Get_P_Critical();
    if (p_critical < 0)
    {
        std::cerr << "p_critical (" << p_critical << ") must be a positive number\n";
        return 1;
    }

    std::cout << "The program is running in '" << arg_parser.Get_Run_Type_Str() << "' mode\n\n";
    
    auto resource_manager = kiv_ppr::Singleton<kiv_ppr::CResource_Manager>::Get_Instance();
    resource_manager->Find_Available_GPUs();
    resource_manager->Print_Available_GPUs();

    const auto seconds = kiv_ppr::utils::Time_Call([&]() {
        Run(arg_parser.Get_Filename(), p_critical);
    });
    std::cout << "\nTime of execution: " << seconds << " sec\n";

    std::cin.get();
}