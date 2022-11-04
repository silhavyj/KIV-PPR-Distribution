#include <iostream>
#include <string.h>

#include "gpu_kernels.h"

namespace kiv_ppr::kernels
{
    TOpenCL_Settings Init_OpenCL(const cl::Device* device, const char* src, const char* kernel_name)
    {
        cl::Program::Sources sources(1, { src, strlen(src) + 1 });
        cl::Context context(*device);
        cl::Program program(context, sources);

        try
        {
            program.build("-cl-std=CL2.0");
            cl::Kernel kernel(program, kernel_name);
            const size_t work_group_size = kernel.getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(*device);

            return { program, context, device, kernel, work_group_size };
        }
        catch (const cl::Error& e)
        {
            std::cout << "OpenCL Error (program.build): " << e.what() << std::endl;
            std::exit(6);
        }
    }
}