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
            const size_t local_mem_size = device->getInfo<CL_DEVICE_LOCAL_MEM_SIZE>();

            return { program, context, device, kernel, work_group_size, local_mem_size };
        }
        catch (const cl::Error& e)
        {
            Print_OpenCL_Error(e, *device);
            std::exit(6);
        }
    }

    inline void Print_OpenCL_Error(const cl::Error& e, const cl::Device& device)
    {
        std::string device_name = device.getInfo<CL_DEVICE_NAME>();
        device_name.pop_back();
        const char* error_desc = Get_OpenCL_Error_Desc(e.err());

        std::cout << "OpenCL Error [" << device_name << "] (" << error_desc << "): " << e.what() << std::endl;
    }

    void Adjust_Work_Group_Size(kernels::TOpenCL_Settings& opencl, size_t size_of_local_params)
    {
        while (opencl.work_group_size * size_of_local_params > opencl.local_mem_size)
        {
            opencl.work_group_size /= 2;
        }
        if (0 == opencl.work_group_size)
        {
            std::string device_name = opencl.device->getInfo<CL_DEVICE_NAME>();
            device_name.pop_back();
            std::cout << "OpenCL Error [" << device_name << "]: " << "local memory size (" << opencl.local_mem_size << " B) is not sufficient for the kernel to store all __local parameters" << std::endl;
            std::exit(9);
        }
    }

    const char* Get_OpenCL_Error_Desc(cl_int error)
    {
        switch (error) 
        {
            case 0: return "CL_SUCCESS";
            case -1: return "CL_DEVICE_NOT_FOUND";
            case -2: return "CL_DEVICE_NOT_AVAILABLE";
            case -3: return "CL_COMPILER_NOT_AVAILABLE";
            case -4: return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
            case -5: return "CL_OUT_OF_RESOURCES";
            case -6: return "CL_OUT_OF_HOST_MEMORY";
            case -7: return "CL_PROFILING_INFO_NOT_AVAILABLE";
            case -8: return "CL_MEM_COPY_OVERLAP";
            case -9: return "CL_IMAGE_FORMAT_MISMATCH";
            case -10: return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
            case -11: return "CL_BUILD_PROGRAM_FAILURE";
            case -12: return "CL_MAP_FAILURE";
            case -13: return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
            case -14: return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
            case -15: return "CL_COMPILE_PROGRAM_FAILURE";
            case -16: return "CL_LINKER_NOT_AVAILABLE";
            case -17: return "CL_LINK_PROGRAM_FAILURE";
            case -18: return "CL_DEVICE_PARTITION_FAILED";
            case -19: return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";

            // compile-time errors
            case -30: return "CL_INVALID_VALUE";
            case -31: return "CL_INVALID_DEVICE_TYPE";
            case -32: return "CL_INVALID_PLATFORM";
            case -33: return "CL_INVALID_DEVICE";
            case -34: return "CL_INVALID_CONTEXT";
            case -35: return "CL_INVALID_QUEUE_PROPERTIES";
            case -36: return "CL_INVALID_COMMAND_QUEUE";
            case -37: return "CL_INVALID_HOST_PTR";
            case -38: return "CL_INVALID_MEM_OBJECT";
            case -39: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
            case -40: return "CL_INVALID_IMAGE_SIZE";
            case -41: return "CL_INVALID_SAMPLER";
            case -42: return "CL_INVALID_BINARY";
            case -43: return "CL_INVALID_BUILD_OPTIONS";
            case -44: return "CL_INVALID_PROGRAM";
            case -45: return "CL_INVALID_PROGRAM_EXECUTABLE";
            case -46: return "CL_INVALID_KERNEL_NAME";
            case -47: return "CL_INVALID_KERNEL_DEFINITION";
            case -48: return "CL_INVALID_KERNEL";
            case -49: return "CL_INVALID_ARG_INDEX";
            case -50: return "CL_INVALID_ARG_VALUE";
            case -51: return "CL_INVALID_ARG_SIZE";
            case -52: return "CL_INVALID_KERNEL_ARGS";
            case -53: return "CL_INVALID_WORK_DIMENSION";
            case -54: return "CL_INVALID_WORK_GROUP_SIZE";
            case -55: return "CL_INVALID_WORK_ITEM_SIZE";
            case -56: return "CL_INVALID_GLOBAL_OFFSET";
            case -57: return "CL_INVALID_EVENT_WAIT_LIST";
            case -58: return "CL_INVALID_EVENT";
            case -59: return "CL_INVALID_OPERATION";
            case -60: return "CL_INVALID_GL_OBJECT";
            case -61: return "CL_INVALID_BUFFER_SIZE";
            case -62: return "CL_INVALID_MIP_LEVEL";
            case -63: return "CL_INVALID_GLOBAL_WORK_SIZE";
            case -64: return "CL_INVALID_PROPERTY";
            case -65: return "CL_INVALID_IMAGE_DESCRIPTOR";
            case -66: return "CL_INVALID_COMPILER_OPTIONS";
            case -67: return "CL_INVALID_LINKER_OPTIONS";
            case -68: return "CL_INVALID_DEVICE_PARTITION_COUNT";

            // extension errors
            case -1000: return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
            case -1001: return "CL_PLATFORM_NOT_FOUND_KHR";
            case -1002: return "CL_INVALID_D3D10_DEVICE_KHR";
            case -1003: return "CL_INVALID_D3D10_RESOURCE_KHR";
            case -1004: return "CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR";
            case -1005: return "CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR";

            default: return "Unknown OpenCL error";
        }
    }
}