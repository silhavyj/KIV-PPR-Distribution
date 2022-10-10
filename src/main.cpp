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

// Compute c = a + b.
static const char source[] =
"#if defined(cl_khr_fp64)\n"
"#  pragma OPENCL EXTENSION cl_khr_fp64: enable\n"
"#elif defined(cl_amd_fp64)\n"
"#  pragma OPENCL EXTENSION cl_amd_fp64: enable\n"
"#else\n"
"#  error double precision is not supported\n"
"#endif\n"
"kernel void add(\n"
"       ulong n,\n"
"       global const double *a,\n"
"       global const double *b,\n"
"       global double *c\n"
"       )\n"
"{\n"
"    size_t i = get_global_id(0);\n"
"    if (i < n) {\n"
"       c[i] = a[i] + b[i];\n"
"    }\n"
"}\n";

void OpenCL_Test()
{
   /* std::vector<cl::Platform> platforms;
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

    cl::Context context;
    cl::CommandQueue queue(context, available_devices.front());
    cl::Program program(context, cl::Program::Sources(1, std::make_pair(source, strlen(source))));

    auto error = program.build("-cl-std=CL1.2");

    cl::Kernel add(program, "add", &error);

    const size_t N = 1024;

    std::vector<double> a(N, 1);
    std::vector<double> b(N, 2);
    std::vector<double> c(N);

    cl::Buffer A(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, a.size() * sizeof(double), a.data());
    cl::Buffer B(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, b.size() * sizeof(double), b.data());
    cl::Buffer C(context, CL_MEM_READ_WRITE, c.size() * sizeof(double));

    add.setArg(0, static_cast<cl_ulong>(N));
    add.setArg(1, A);
    add.setArg(2, B);
    add.setArg(3, C);

    queue.enqueueNDRangeKernel(add, cl::NullRange, N, cl::NullRange);
    queue.enqueueReadBuffer(C, CL_TRUE, 0, c.size() * sizeof(double), c.data());

    std::cout << "C[42]=" << c[42] << "\n"; */

    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    auto platform = platforms.front();
    std::vector<cl::Device> devices;
    platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);

    auto device = devices.front();

    std::ifstream helloWorldFile("..\\src\\kernels\\kernel.cl");
    std::string src(std::istreambuf_iterator<char>(helloWorldFile), (std::istreambuf_iterator<char>()));

    cl::Program::Sources sources(1, std::make_pair(src.c_str(), src.length() + 1));

    cl::Context context(device);
    cl::Program program(context, sources);

    auto err = program.build("-cl-std=CL1.2");

    char buf[16];
    cl::Buffer memBuf(context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, sizeof(buf));
    cl::Kernel kernel(program, "HelloWorld", &err);
    kernel.setArg(0, memBuf);

    cl::CommandQueue queue(context, device);
    queue.enqueueTask(kernel);
    queue.enqueueReadBuffer(memBuf, GL_TRUE, 0, sizeof(buf), buf);

    std::cout << "buffer=" << buf << "\n";
}

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    //OpenCL_Test();

    // kiv_ppr::utils::Generate_Numbers<std::poisson_distribution<>>(filename.c_str(), true, 10000, 1000);

    const auto seconds = kiv_ppr::utils::Time_Call([]() {
        Run();
    });
    std::cout << " (" << seconds << " sec)\n";

    // kiv_ppr::CPoisson_CDF c(1000);
    // std::cout << "A=" << c(1000) << '\n';

    std::cin.get();
}