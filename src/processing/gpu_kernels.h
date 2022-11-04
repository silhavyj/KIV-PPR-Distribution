#pragma once

// https://stackoverflow.com/questions/56858213/how-to-create-nvidia-opencl-project/57017982#57017982
#define CL_USE_DEPRECATED_OPENCL_2_0_APIS
#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>

namespace kiv_ppr::kernels
{
    static constexpr const char* First_Iteration_Kernel_Name = "First_File_Iteration\0";

    static constexpr const char* First_Iteration_Kernel = 
        "#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n"
        "#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable\n"
        "\n"
        "// https://opensource.apple.com/source/Libm/Libm-315/Source/ARM/fpclassify.c.auto.html\n"
        "int fpclassify(double x)\n"
        "{\n"
        "    ulong u = *(ulong*)&x;\n"
        "    uint exp = (uint)((u & 0x7fffffffffffffffUL) >> 52);\n"
        "\n"
        "    if (0 == exp)\n"
        "    {\n"
        "        if (u & 0x000fffffffffffffUL)\n"
        "        {\n"
        "            return 0;\n"
        "        }\n"
        "        return 1;\n"
        "    }\n"
        "    if (0x7ff == exp)\n"
        "    {\n"
        "        if (u & 0x000fffffffffffffUL)\n"
        "        {\n"
        "            return 2;\n"
        "        }\n"
        "        return 3;\n"
        "    }\n"
        "    return 4;\n"
        "}\n"
        "\n"
        "int Is_Valid_Double(double x)\n"
        "{\n"
        "    int validation_result = fpclassify(x);\n"
        "    return 4 == validation_result || 1 == validation_result;\n"
        "}\n"
        "\n"
        "__kernel void First_File_Iteration(__global double* data,\n"
        "                                   __local double* local_mean,\n"
        "                                   __global double* out_mean,\n"
        "                                   __local double* local_min,\n"
        "                                   __global double* out_min,\n"
        "                                   __local double* local_max,\n"
        "                                   __global double* out_max,\n"
        "                                   __global ulong* out_count,\n"
        "                                   __global ulong* out_all_ints)\n"
        "{\n"
        "    size_t global_id = get_global_id(0);\n"
        "    size_t local_id = get_local_id(0);\n"
        "    size_t local_size = get_local_size(0);\n"
        "    size_t group_id = get_group_id(0);\n"
        "\n"
        "    local_mean[local_id] = data[global_id] / 2.0;\n"
        "    local_min[local_id] = local_mean[local_id];\n"
        "    local_max[local_id] = local_mean[local_id];\n"
        "\n"
        "    barrier(CLK_LOCAL_MEM_FENCE);\n"
        "\n"
        "    if (Is_Valid_Double(data[global_id]))\n"
        "    {\n"
        "        atom_inc(&out_count[0]);\n"
        "\n"
        "        if (ceil(data[global_id]) != floor(data[global_id]))\n"
        "        {\n"
        "            atom_inc(&out_all_ints[0]);\n"
        "        }\n"
        "    }\n"
        "\n"
        "    int valid_1;\n"
        "    int valid_2;\n"
        "\n"
        "    for (size_t i = local_size / 2; i > 0; i /= 2)\n"
        "    {\n"
        "        if (local_id < i)\n"
        "        {\n"
        "            valid_1 = Is_Valid_Double(local_mean[local_id]);\n"
        "            valid_2 = Is_Valid_Double(local_mean[local_id + i]);\n"
        "\n"
        "            if (!valid_1 && valid_2)\n"
        "            {\n"
        "                local_mean[local_id] = local_mean[local_id + i];\n"
        "                local_min[local_id] = local_min[local_id + i];\n"
        "                local_max[local_id] = local_max[local_id + i];\n"
        "                valid_1 = 1;\n"
        "            }\n"
        "            if (valid_1 && valid_2)\n"
        "            {\n"
        "                local_mean[local_id] = (local_mean[local_id] / 2.0) + (local_mean[local_id + i] / 2.0);\n"
        "                local_min[local_id] = min(local_min[local_id], local_min[local_id + i]);\n"
        "                local_max[local_id] = max(local_max[local_id], local_max[local_id + i]);\n"
        "            }\n"
        "        }\n"
        "\n"
        "        barrier(CLK_LOCAL_MEM_FENCE);\n"
        "    }\n"
        "\n"
        "    if (0 == local_id)\n"
        "    {\n"
        "        out_mean[group_id] = local_mean[0];\n"
        "        out_min[group_id] = local_min[0];\n"
        "        out_max[group_id] = local_max[0];\n"
        "    }\n"
        "}\n";

    struct TOpenCL_Settings
    {
        cl::Program program;
        cl::Context context;
        const cl::Device* device;
        cl::Kernel kernel;
        size_t work_group_size;
    };

    TOpenCL_Settings Init_OpenCL(const cl::Device* device, const char* src, const char* kernel_name);
}