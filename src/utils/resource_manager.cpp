#include <iostream>

#include "resource_manager.h"

namespace kiv_ppr
{
    CResource_Manager::TResource::TResource(NDevice_Status status, cl::Device device)
        : m_status(status),
          m_device(device)
    {

    }

    void CResource_Manager::Set_Run_Type(CArg_Parser::NRun_Type run_type) noexcept
    {
        m_run_type = run_type;
    }

    void CResource_Manager::Find_Available_GPUs(const std::unordered_set<std::string>& listed_devices)
    {
        if (m_run_type == CArg_Parser::NRun_Type::SMP)
        {
            return;
        }

        // Find all platforms on the machine.
        std::vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);
	
        // Found devices on the machine (they all must be 
        // available and support double precision).
        std::unordered_set<std::string> found_devices;

        for (const auto& platform : platforms)
        {
            // Get all devices of the current platform.
            std::vector<cl::Device> devices;
            platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);

            for (const auto& device : devices)
            {
                // Check if the device is available.
                if (!device.getInfo<CL_DEVICE_AVAILABLE>())
                {
                    continue;
                }
				
                // Retrieve the device's name.
                std::string name = device.getInfo<CL_DEVICE_NAME>();
                name.pop_back();

                // Retrieve the device's extensions (what features it supports).
                const std::string extensions = device.getInfo<CL_DEVICE_EXTENSIONS>();

                // Check if the device supports double precision.
                if (extensions.find("cl_khr_fp64") == std::string::npos &&
                    extensions.find("cl_amd_fp64") == std::string::npos)
                {
                    continue;
                }

                // Based on the mode of the program, add the device to the 
                // list of OpenCL devices to be used by the program.
                if ((m_run_type == CArg_Parser::NRun_Type::OpenCL_Devs && listed_devices.count(name)) ||
                     m_run_type == CArg_Parser::NRun_Type::All)
                {
                    m_gpu_devices.emplace_back(NDevice_Status::Available, device);
                }

                // Add the device to the list of all existing devices.
                found_devices.insert(name);
            }
        }

        // Print out OpenCL devices to be used by the program.
        Print_Found_Devs(found_devices, listed_devices);
    }

    void CResource_Manager::Print_Found_Devs(const std::unordered_set<std::string>& found_devices,
                                             const std::unordered_set<std::string>& listed_devices)
    {
        if (m_run_type == CArg_Parser::NRun_Type::All)
        {
            // No OpenCL devices were found.
            if (m_gpu_devices.empty())
            {
                std::cout << "No available OpenCL devices supported double precision were found" << std::endl;
                return;
            }

            // Print out all OpenCL devices that were found on the machine.
            std::cout << "Available OpenCL devices supporting double precision:" << std::endl;
            for (const auto& [status, device] : m_gpu_devices)
            {
                std::cout << device.getInfo<CL_DEVICE_NAME>() << std::endl;
            }
            std::cout << std::endl;
        }
        else
        {
            // Check if all of the listed devices were found on the machine.
            std::cout << "Checking availability of the listed devices:" << std::endl;
            bool terminate_program = false;
            for (const auto& device_name : listed_devices)
            {
                std::cout << "Name: " << device_name << " (";
                if (!found_devices.count(device_name))
                {
                    std::cout << "ERR)" << std::endl;
                    terminate_program = true;
                }
                else
                {
                    std::cout << "OK)" << std::endl;
                }
            }
            std::cout << std::endl;
            if (terminate_program)
            {
                std::cout << "Some of the listed OpenCL devices are not available or do not support double precision" << std::endl;
                std::exit(6);
            }
        }
    }

    const cl::Device* CResource_Manager::Get_Available_Device()
    {
        const std::lock_guard<std::mutex> lock(m_mtx);
        for (auto& [status, device] : m_gpu_devices)
        {
            if (NDevice_Status::Available == status)
            {
                status = NDevice_Status::Taken;
                return &device;
            }
        }
        return nullptr;
    }

    void CResource_Manager::Release_Device(const cl::Device* device)
    {
        const std::lock_guard<std::mutex> lock(m_mtx);
        for (auto& [status, m_device] : m_gpu_devices)
        {
            if (&m_device == device)
            {
                status = NDevice_Status::Available;
            }
        }
    }

    CArg_Parser::NRun_Type CResource_Manager::Get_Run_Type() const noexcept
    {
        return m_run_type;
    }
}

// EOF