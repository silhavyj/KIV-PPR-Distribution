#pragma once

// How to set up an OpenCL project:
// https://stackoverflow.com/questions/56858213/how-to-create-nvidia-opencl-project/57017982#57017982

#define CL_USE_DEPRECATED_OPENCL_2_0_APIS
#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>

namespace kiv_ppr::kernels
{
    /// Name of the entry point ("main function") of the kernel performing the first iteration.
    static constexpr const char* First_Iteration_Kernel_Name = "First_File_Iteration";

    /// Size of the local parameters the kernel takes as parameters.
    /// This values is then multiplied by the work group size, to find out
    /// if the maximum local memory size is exceeded or not.
    static constexpr size_t First_Iteration_Get_Size_Of_Local_Params = 3 * sizeof(double) + sizeof(int) + sizeof(cl_ulong);

    /// Kernel for the first iteration.
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
                                           __local ulong* local_count,
                                          
                                           __global double* out_min,
                                           __global double* out_max,
                                           __global double* out_mean,
                                           __global int* out_all_ints,
                                           __global ulong* out_count)
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

    /// Name of the entry point ("main function") of the kernel performing the second iteration.
    static constexpr const char* Second_Iteration_Kernel_Name = "Second_File_Iteration";

    /// Size of the local parameters the kernel takes as parameters.
    /// This values is then multiplied by the work group size, to find out
    /// if the maximum local memory size is exceeded or not.
    static constexpr size_t Second_Iteration_Get_Size_Of_Local_Params = 1 * sizeof(double);

    /// Kernel for the second iteration.
    static constexpr const char* Second_Iteration_Kernel = R"CLC(
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

        __kernel void Second_File_Iteration(__global double* data,
                                            __local double* local_var,
                                            __global double* out_var,
                                            __global uint* histogram,
                                            double mean,
                                            ulong count,
                                            double min,
                                            double interval_size)
        {
            size_t global_id = get_global_id(0);
            size_t local_id = get_local_id(0);
            size_t local_size = get_local_size(0);
            size_t group_id = get_group_id(0);

            double value = data[global_id];
            int is_valid = Is_Valid_Double(value);

            local_var[local_id] = value;
            if (!is_valid)
            {
                local_var[local_id] = 0;
            }

            if (is_valid)
            {
                if (min < 0)
                {
                    value /= 2;
                }

                size_t slot_id = (size_t)((value - min) / interval_size);

                uint old_value = atomic_inc(&histogram[2 * slot_id]);
                uint carry = old_value == 0xFFFFFFFF;
                atomic_add(&histogram[2 * slot_id + 1], carry);

                double delta = value - mean;
                double tmp_val = delta;
                delta /= (count - 1);
                delta *= tmp_val;

                local_var[local_id] = delta;
            }

            barrier(CLK_LOCAL_MEM_FENCE);

            for (size_t i = local_size / 2; i > 0; i /= 2)
            {
                if (local_id < i)
                {
                    local_var[local_id] += local_var[local_id + i];
                }

                barrier(CLK_LOCAL_MEM_FENCE);
            }

            if (0 == local_id)
            {
                out_var[group_id] = local_var[0];
            }
        }
    )CLC";

    /// Data associated with an OpenCL device.
    struct TOpenCL_Settings
    {
        cl::Program program{};              ///< Program the device will execute
        cl::Context context{};              ///< Context within which the code will execute
        const cl::Device* device = nullptr; ///< OpenCL device itself
        cl::Kernel kernel{};                ///< Kernel ("source code")
        size_t work_group_size = 0;         ///< Maximum work group size of the device
        size_t local_mem_size = 0;          ///< Maximum local memory size of the device
    };

    /// Initializes an OpenCL device and creates a kernel.
    /// This method is called from a worker thread after it gets hold of an available OpenCL device.
    /// \param device OpenCL device
    /// \param src Kernel source code
    /// \param kernel_name Name of the kernel's entry point
    /// \return Data associated with the OpenCL device and the kernel.
    [[nodiscard]] TOpenCL_Settings Init_OpenCL(const cl::Device* device, const char* src, const char* kernel_name);

    /// Helper function that returns a text description based on an OpenCL error code.
    /// @param error OpenCL error code
    /// @return Text description of the error.
    [[nodiscard]] const char* Get_OpenCL_Error_Desc(cl_int error);

    /// Prints out an OpenCL error.
    /// \param e OpenCL error that has been caught
    /// \param device OpenCL device associated with the error.
    void Print_OpenCL_Error(const cl::Error& e, const cl::Device& device);

    /// Modifies the work group size of an OpenCL device, so it is able to execute the given kernel.
    /// We need to make sure that the total size of __local atributes does not exceed 
    /// the maximum local memory size of the device. If it does, we need to device the work group size
    /// by 2 until the condition is satisfied. The work group size has to be a power of two number,
    /// so the numerical reduce algorithm works properly.
    /// \param opencl OpenCL device 
    /// \param size_of_local_params Size of the local values the device takes as parameters.
    void Adjust_Work_Group_Size(kernels::TOpenCL_Settings& opencl, size_t size_of_local_params);
}

// EOF