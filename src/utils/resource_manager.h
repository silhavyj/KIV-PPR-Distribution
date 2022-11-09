#pragma once

#include <vector>
#include <mutex>
#include <unordered_set>

// https://stackoverflow.com/questions/56858213/how-to-create-nvidia-opencl-project/57017982#57017982
#define CL_USE_DEPRECATED_OPENCL_2_0_APIS
#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>

#include "arg_parser.h"

namespace kiv_ppr
{
    class CResource_Manager
    {
    public:
        CResource_Manager() = default;
        ~CResource_Manager() = default;

        void Set_Run_Type(CArg_Parser::NRun_Type run_type);
        void Find_Available_GPUs(const std::unordered_set<std::string>& listed_devices);
        [[nodiscard]] const cl::Device* Get_Available_Device();
        void Release_Device(const cl::Device* device);
        [[nodiscard]] CArg_Parser::NRun_Type Get_Run_Type() const;


    private:
        void Print_Found_Devs(const std::unordered_set<std::string>& found_devices,
                              const std::unordered_set<std::string>& listed_devices);

    private:
        enum class NDevice_Status : uint8_t
        {
            Available,
            Taken
        };

        struct TRecource
        {
            NDevice_Status m_status;
            cl::Device m_device;

            TRecource(NDevice_Status status, cl::Device device);
        };

    private:
        std::vector<TRecource> m_gpu_devices;
        CArg_Parser::NRun_Type m_run_type{};
        std::mutex m_mtx;
    };
}