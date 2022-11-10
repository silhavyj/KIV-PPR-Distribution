#include <iostream>

#include "utils/utils.h"
#include "utils/arg_parser.h"
#include "utils/resource_manager.h"
#include "utils/file_reader.h"
#include "utils/singleton.h"
#include "config.h"
#include "processing/file_stats.h"
#include "chi_square/test_runner.h"

/// Runs the program. It processes the input file and based on 
/// the statistical values calculated in both iterations, it runs
/// the Chi-Square goodness of fit test and prints out the results.
/// \param filename Path to the input file
/// \param p_critical Critical P-value used in the statistical tests.
static void Run(const char* filename, double p_critical)
{
    // Create a file reader.
    kiv_ppr::CFile_Reader<double> file(filename);

    if (file.Is_Open())
    {
        // The input fil has to contain at least one double.
        if (file.Get_File_Size() < sizeof(double))
        {
            std::cout << "The size of the input file is insufficient" << std::endl;
            std::exit(1);
        }

        // Print out information for the user.
        std::cout << "Processing file " << file.Get_Filename() << " [" << file.Get_File_Size() << " B]" << std::endl;

        // Process the input file (calculate min, max, mean, histogram, ...).
        kiv_ppr::CFile_Stats file_stats(&file);
        if (0 != file_stats.Process(&kiv_ppr::config::default_thread_params))
        {
            std::cout << "Failed to process the input file (" << file.Get_Filename() << ")" << std::endl;
            std::exit(1);
        }

        // Print out the values calculated from the input file.
        auto values = file_stats.Get_Values();
        std::cout << "\nCalculated statistics (parameters):" << std::endl;
        std::cout << values << "\n" << std::endl;

        // Run the statistical tests.
        kiv_ppr::CTest_Runner test_runner(values, p_critical);
        test_runner.Run();
    }
    else
    {
        std::cout << "Failed to open the input file (" << file.Get_Filename() << ")" << std::endl;
        std::exit(1);
    }
}

/// Entry point of the program
/// \param argc Number of parameters passed in from the command line
/// \param argv Arguments from the command line
/// \return Execution status code
int main(int argc, char* argv[])
{
    // Just for testing
    // kiv_ppr::utils::Generate_Numbers<std::normal_distribution<>>("test_data.dat", true, 17179869184 / sizeof(double), 2, 5);

    // Parse input arguments.
    kiv_ppr::CArg_Parser arg_parser(argc, argv);
    try
    {
        arg_parser.Parse_Options();

        // Check if the user entered '--help' or '-h'.
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

    // Get the critical P-value and make sure that it falls into the valid range <0; 1>.
    double p_critical = arg_parser.Get_P_Critical();
    if (p_critical < 0.0 || p_critical > 1.0)
    {
        std::cout << "p_critical (" << p_critical << ") must be a positive number <0; 1>" << std::endl;
        return 1;
    }

    // Set up thread configuration based on what the user entered into the program.
    kiv_ppr::config::default_thread_params.number_of_elements_per_file_read = arg_parser.Get_Block_Size_Per_Read();
    kiv_ppr::config::default_thread_params.watchdog_expiration_sec = arg_parser.Get_Watchdog_Sleep_Sec();
    kiv_ppr::config::default_thread_params.number_of_threads = arg_parser.Get_Number_Of_Threads();

    // Print out info as to how the program is going to be executed.
    std::cout << "The program is running in '" << arg_parser.Get_Run_Type_Str() << "' mode" << std::endl;
    std::cout << "Block size per read = " << kiv_ppr::config::default_thread_params.number_of_elements_per_file_read << " [B]" << std::endl;
    std::cout << "Watchdog checkup period = " << kiv_ppr::config::default_thread_params.watchdog_expiration_sec << "s" << std::endl;
    std::cout << "Number of threads = " << kiv_ppr::config::default_thread_params.number_of_threads << std::endl << std::endl;
    
    // Get a list of  the OpenCL devices the user wishes to use.
    const auto& listed_devs = arg_parser.Get_OpenCL_Devs();

    auto resource_manager = kiv_ppr::Singleton<kiv_ppr::CResource_Manager>::Get_Instance();
    if (nullptr == resource_manager)
    {
        std::cout << "Error: resource manager is NULL" << std::endl;
        std::exit(21);
    }

    // Set the mode of the program (smp, all, ...).
    resource_manager->Set_Run_Type(arg_parser.Get_Run_Type());

    // Check out the availability of the listed OpenCL devices.
    resource_manager->Find_Available_GPUs(listed_devs);

    // Run the program.
    const auto seconds = kiv_ppr::utils::Time_Call([&]() {
        Run(arg_parser.Get_Filename(), p_critical);
    });

    // Print out how much time it took to process the input file a run the statistical tests.
    std::cout << "\nTime of execution: " << seconds << " sec" << std::endl;

    // Just for testing
    // std::cin.get();
}

// EOF