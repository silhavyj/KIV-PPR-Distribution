#pragma once

// https://stackoverflow.com/questions/56858213/how-to-create-nvidia-opencl-project/57017982#57017982
#define CL_USE_DEPRECATED_OPENCL_2_0_APIS
#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>

namespace kiv_ppr::kernels
{
    static constexpr const char* First_Iteration_Kernel_Name = "First_File_Iteration";

    static constexpr size_t First_Iteration_Get_Size_Of_Local_Params = 4 * sizeof(double) + sizeof(int);

    static constexpr const char* First_Iteration_Kernel = R"CLC(
        #pragma OPENCL EXTENSION cl_khr_fp64 : enable

        int Is_Valid_Double(double x)
        {
            ulong u = *(ulong*)&x;
            uint exp = (uint)((u & 0x7fffffffffffffffUL) >> 52);

            if (0 == exp)
            {
                if (u & 0x000fffffffffffffUL)
                {
                    return 0;
                }
                return 1;
            }
            if (0x7ff == exp)
            {
                if (u & 0x000fffffffffffffUL)
                {
                    return 0;
                }
                return 0;
            }
            return 1;
        }

        __kernel void First_File_Iteration(__global double* data,

                                           __local double* local_min,
                                           __local double* local_max,
                                           __local double* local_mean,
                                           __local int* local_all_ints,
                                           __local double* local_count,
                                          
                                           __global double* out_min,
                                           __global double* out_max,
                                           __global double* out_mean,
                                           __global int* out_all_ints,
                                           __global double* out_count)
        {
            size_t global_id = get_global_id(0);
            size_t local_id = get_local_id(0);
            size_t local_size = get_local_size(0);
            size_t group_id = get_group_id(0);

            local_mean[local_id] = data[global_id] / 2.0;
            local_min[local_id] = local_mean[local_id];
            local_max[local_id] = local_mean[local_id];
            local_count[local_id] = Is_Valid_Double(data[global_id]);
            local_all_ints[local_id] = !local_count[local_id] || ceil(data[global_id]) == floor(data[global_id]);

            barrier(CLK_LOCAL_MEM_FENCE);

            int valid_1;
            int valid_2;

            for (size_t i = local_size / 2; i > 0; i /= 2)
            {
                if (local_id < i)
                {
                    valid_1 = Is_Valid_Double(local_mean[local_id]);
                    valid_2 = Is_Valid_Double(local_mean[local_id + i]);

                    local_count[local_id] += local_count[local_id + i];
                    local_all_ints[local_id] = local_all_ints[local_id] && local_all_ints[local_id + i];

                    if (!valid_1 && valid_2)
                    {
                        local_mean[local_id] = local_mean[local_id + i];
                        local_min[local_id] = local_min[local_id + i];
                        local_max[local_id] = local_max[local_id + i];
                        valid_1 = 1;
                    }
                    if (valid_1 && valid_2)
                    {
                        local_mean[local_id] = (local_mean[local_id] / 2.0) + (local_mean[local_id + i] / 2.0);
                        local_min[local_id] = min(local_min[local_id], local_min[local_id + i]);
                        local_max[local_id] = max(local_max[local_id], local_max[local_id + i]);
                    }
                }

                barrier(CLK_LOCAL_MEM_FENCE);
            }

            if (0 == local_id)
            {
                out_mean[group_id] = local_mean[0];
                out_min[group_id] = local_min[0];
                out_max[group_id] = local_max[0];
                out_count[group_id] = local_count[0];
                out_all_ints[group_id] = local_all_ints[0];
            }
        }
    )CLC";

    struct TOpenCL_Settings
    {
        cl::Program program;
        cl::Context context;
        const cl::Device* device;
        cl::Kernel kernel;
        size_t work_group_size;
        size_t local_mem_size;
    };

    TOpenCL_Settings Init_OpenCL(const cl::Device* device, const char* src, const char* kernel_name);
    const char* Get_OpenCL_Error_Desc(cl_int error);
    void Print_OpenCL_Error(const cl::Error& e, const cl::Device& device);
}