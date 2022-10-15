#include <iostream>

#include "ResourceManager.h"

namespace kiv_ppr
{
    void CResource_Manager::Set_Run_Type(CArg_Parser::NRun_Type run_type)
    {
        m_run_type = run_type;
    }

    void CResource_Manager::Find_Available_GPUs(const std::unordered_set<std::string>& listed_devices)
    {
        if (m_run_type == CArg_Parser::NRun_Type::SMP)
        {
            return;
        }

        std::vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);
	
        std::unordered_set<std::string> found_devices;
        for (const auto& platform : platforms)
        {
            std::vector<cl::Device> devices;
            platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
            for (const auto& device : devices)
            {
                if (!device.getInfo<CL_DEVICE_AVAILABLE>())
                {
                    continue;
                }
				
                const std::string name = device.getInfo<CL_DEVICE_NAME>();
                const std::string extensions = device.getInfo<CL_DEVICE_EXTENSIONS>();

                if (extensions.find("cl_khr_fp64") == std::string::npos &&
                    extensions.find("cl_amd_fp64") == std::string::npos)
                {
                    continue;
                }
                if ((m_run_type == CArg_Parser::NRun_Type::OPENCL_DEVS && listed_devices.count(name)) ||
                     m_run_type == CArg_Parser::NRun_Type::ALL)
                {
                    m_gpu_devices.emplace_back(NDevice_Status::AVAILABLE, device);
                }
                found_devices.insert(device.getInfo<CL_DEVICE_NAME>());
            }
        }
        Print_Found_Devs(found_devices, listed_devices);
    }

    void CResource_Manager::Print_Found_Devs(const std::unordered_set<std::string>& found_devices,
                                             const std::unordered_set<std::string>& listed_devices)
    {
        if (m_run_type == CArg_Parser::NRun_Type::ALL)
        {
            if (m_gpu_devices.empty())
            {
                std::cerr << "No available GPU devices supported double precision were found\n";
                std::exit(6);
            }
            std::cout << "Available GPU devices supporting double precision:\n";
            for (const auto& [status, device] : m_gpu_devices)
            {
                std::cout << device.getInfo<CL_DEVICE_NAME>() << '\n';
            }
            std::cout << '\n';
        }
        else
        {
            std::cout << "Checking availability of the listed devices:\n";
            bool terminate_program = false;
            for (const auto& device_name : listed_devices)
            {
                std::cout << "Name: " << device_name << " (";
                if (!found_devices.count(device_name))
                {
                    std::cout << "ERR)\n";
                    terminate_program = true;
                }
                else
                {
                    std::cout << "OK)\n";
                }
            }
            std::cout << '\n';
            if (terminate_program)
            {
                std::cerr << "Some of the listed devices are not available or do not support double precision\n";
                std::exit(6);
            }
        }
    }
}