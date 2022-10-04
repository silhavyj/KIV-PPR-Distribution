#include <iostream>
#include <thread>

#include "utils/Utils.h"

#include "Config.h"
#include "FileReader.h"
#include "processing/FileStats.h"
#include "tests/TestRunner.h"

#define CL_USE_DEPRECATED_OPENCL_2_0_APIS
#include <CL/cl.hpp>

#include "cdfs/PoissonCDF.h"

// static std::string filename{"..\\referencni_rozdeleni\\uniform"};
static std::string filename{ "data.dat" };

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

        std::cout << "\nRunning Chi Square statistical test...\n\n";
        kiv_ppr::CTest_Runner test_runner(values);
        test_runner.Run();
    }
    else
    {
        std::cerr << "Failed to open the input file\n";
        std::exit(1);
    }
}

void OpenCL_Test()
{
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    std::vector<cl::Device> available_devices;

    if (platforms.empty())
    {
        std::cerr << "OpenCL platforms not found\n";
        std::exit(2);
    }

    for (const auto& platform : platforms)
    {
        std::vector<cl::Device> devices;
        platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);

        for (const auto& device : devices)
        {
            if (!device.getInfo<CL_DEVICE_AVAILABLE>())
            {
                continue;
            }
           
            std::string extensions = device.getInfo<CL_DEVICE_EXTENSIONS>();
            if (extensions.find("cl_khr_fp64") == std::string::npos &&
                extensions.find("cl_amd_fp64") == std::string::npos)
            {
                continue;
            }
            available_devices.push_back(device);
        }
    }

    if (available_devices.empty())
    {
        std::cerr << "GPUs with double precision not found\n";
        std::exit(2);
    }

    std::cout << "GPU devices with double precision:\n";
    for (const auto& device : available_devices)
    {
        std::cout << device.getInfo<CL_DEVICE_VENDOR>() << " (";
        std::cout << device.getInfo<CL_DEVICE_VERSION>() << ")\n";
    }
    std::cout << "\n";
}

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    OpenCL_Test();

    // kiv_ppr::utils::Generate_Numbers<std::poisson_distribution<>>(filename.c_str(), true, 10000, 1000);

    const auto seconds = kiv_ppr::utils::Time_Call([]() {
        Run();
    });
    std::cout << " (" << seconds << " sec)\n";

    kiv_ppr::CPoisson_CDF c(1000);
    std::cout << "A=" << c(1000) << '\n';

    std::cin.get();
}