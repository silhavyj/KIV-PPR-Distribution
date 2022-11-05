#include <iostream>

#include "utils/utils.h"
#include "utils/arg_parser.h"
#include "utils/resource_manager.h"
#include "utils/file_reader.h"
#include "utils/singleton.h"
#include "config.h"
#include "processing/file_stats.h"
#include "chi_square/test_runner.h"

static void Run(const char* filename, double p_critical)
{
    kiv_ppr::CFile_Reader<double> file(filename);
    if (file.Is_Open())
    {
        if (file.Get_File_Size() < sizeof(double))
        {
            std::cout << "The size of the input file is insufficient" << std::endl;
            std::exit(1);
        }

        std::cout << "Processing file " << file.Get_Filename() << " [" << file.Get_File_Size() << " B]" << std::endl;

        kiv_ppr::CFile_Stats file_stats(&file, kiv_ppr::utils::Is_Valid_Double);
        if (0 != file_stats.Process(&kiv_ppr::config::default_thread_params))
        {
            std::cout << "Failed to process the input file (" << file.Get_Filename() << ")" << std::endl;
            std::exit(1);
        }
        auto values = file_stats.Get_Values();

        std::cout << "\nCalculated statistics (parameters):" << std::endl;
        std::cout << values << "\n" << std::endl;

        kiv_ppr::CTest_Runner test_runner(values, p_critical);
        test_runner.Run();
    }
    else
    {
        std::cout << "Failed to open the input file (" << file.Get_Filename() << ")" << std::endl;
        std::exit(1);
    }
}

int main(int argc, char* argv[])
{
    // kiv_ppr::utils::Generate_Numbers<std::poisson_distribution<>>("test_data.dat", true, 104857600 / sizeof(double), 5);

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
        std::cout << "Error occurred when parsing input parameters - " << e.what() << std::endl;
        std::cout << "Run 'pprsolver.exe --help'" << std::endl;
        return 1;
    }

    double p_critical = arg_parser.Get_P_Critical();
    if (p_critical < 0)
    {
        std::cout << "p_critical (" << p_critical << ") must be a positive number" << std::endl;
        return 1;
    }

    std::cout << "The program is running in '" << arg_parser.Get_Run_Type_Str() << "' mode\n" << std::endl;
    
    const auto& listed_devs = arg_parser.Get_OpenCL_Devs();

    auto resource_manager = kiv_ppr::Singleton<kiv_ppr::CResource_Manager>::Get_Instance();
    resource_manager->Set_Run_Type(arg_parser.Get_Run_Type());
    resource_manager->Find_Available_GPUs(listed_devs);

    const auto seconds = kiv_ppr::utils::Time_Call([&]() {
        Run(arg_parser.Get_Filename(), p_critical);
    });
    std::cout << "\nTime of execution: " << seconds << " sec" << std::endl;

    std::cin.get();
}