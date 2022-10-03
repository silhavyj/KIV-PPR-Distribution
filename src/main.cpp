#define CL_USE_DEPRECATED_OPENCL_2_0_APIS

#include <iostream>
#include <thread>

#include "utils/Utils.h"
#include "Config.h"
#include "FileReader.h"
#include "processing/FileStats.h"
#include "tests/TestRunner.h"

#include <CL/cl.hpp>

static std::string filename{"..\\referencni_rozdeleni\\exp"};
// static std::string filename{ "data.dat" };

static void Run()
{
    kiv_ppr::CFile_Reader<double> file(filename);
    if (file.Is_Open())
    {
        std::cout << "Processing file " << file.Get_Filename() << " [" << file.Get_File_Size() << " B]\n\n";

        kiv_ppr::CFile_Stats file_stats(&file, kiv_ppr::utils::Is_Valid_Double);
        if (0 != file_stats.Process(&kiv_ppr::config::default_thread_params))
        {
            std::cerr << "Failed to process the input file\n";
            std::exit(1);
        }
        auto values = file_stats.Get_Values();

        std::cout << "Calculated statistics:\n";
        std::cout << values << '\n';

        // std::cout << *values.second_iteration.histogram << "\n";

        std::cout << "\nRunning Chi Square statistical test...\n";
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

    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    for (const auto& platform : platforms)
    {
        std::vector<cl::Device> devices;
        platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);

        for (const auto& device : devices)
        {
            std::cout << device.getInfo<CL_DEVICE_VENDOR>() << " (";
            std::cout << device.getInfo<CL_DEVICE_VERSION>() << ")\n";
        }
    }


    //kiv_ppr::utils::Generate_Numbers<std::exponential_distribution<>>(filename.c_str(), true, 134217728 , 5);

    const auto seconds = kiv_ppr::utils::Time_Call([]() {
        Run();
    });
    std::cout << " (" << seconds << " sec)\n";

    std::cin.get();
}