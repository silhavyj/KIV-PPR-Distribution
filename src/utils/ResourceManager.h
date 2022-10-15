#pragma once

#include <vector>
#include <unordered_set>

#define CL_USE_DEPRECATED_OPENCL_2_0_APIS
#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_TARGET_OPENCL_VERSION 200
#include <CL/opencl.hpp>

#include "ArgParser.h"

namespace kiv_ppr
{
    class CResource_Manager
    {
    public:
        void Set_Run_Type(CArg_Parser::NRun_Type run_type);
        void Find_Available_GPUs(const std::unordered_set<std::string>& listed_devices);

    private:
        void Print_Found_Devs(const std::unordered_set<std::string>& found_devices,
                              const std::unordered_set<std::string>& listed_devices);

    private:
        enum class NDevice_Status : uint8_t
        {
            AVAILABLE,
            TAKEN
        };

        struct TRecource
        {
            NDevice_Status status;
            cl::Device device;
        };

    private:
        std::vector<TRecource> m_gpu_devices;
        CArg_Parser::NRun_Type m_run_type{};
    };
}