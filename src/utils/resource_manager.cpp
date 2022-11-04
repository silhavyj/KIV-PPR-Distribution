#include <iostream>

#include "resource_manager.h"

namespace kiv_ppr
{
    CResource_Manager::TRecource::TRecource(NDevice_Status status, cl::Device device)
        : m_status(status),
          m_device(device)
    {

    }

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
				
                std::string name = device.getInfo<CL_DEVICE_NAME>();
                name.pop_back();
                const std::string extensions = device.getInfo<CL_DEVICE_EXTENSIONS>();

                if (extensions.find("cl_khr_fp64") == std::string::npos &&
                    extensions.find("cl_amd_fp64") == std::string::npos)
                {
                    continue;
                }
                if ((m_run_type == CArg_Parser::NRun_Type::OpenCL_Devs && listed_devices.count(name)) ||
                     m_run_type == CArg_Parser::NRun_Type::All)
                {
                    m_gpu_devices.emplace_back(NDevice_Status::Available, device);
                }
                std::string device_name = device.getInfo<CL_DEVICE_NAME>();
                device_name.pop_back();
                found_devices.insert(device_name);
            }
        }
        Print_Found_Devs(found_devices, listed_devices);
    }

    void CResource_Manager::Print_Found_Devs(const std::unordered_set<std::string>& found_devices,
                                             const std::unordered_set<std::string>& listed_devices)
    {
        if (m_run_type == CArg_Parser::NRun_Type::All)
        {
            if (m_gpu_devices.empty())
            {
                std::cout << "No available GPU devices supported double precision were found" << std::endl;
                std::exit(6);
            }
            std::cout << "Available GPU devices supporting double precision:" << std::endl;
            for (const auto& [status, device] : m_gpu_devices)
            {
                std::cout << device.getInfo<CL_DEVICE_NAME>() << std::endl;
            }
            std::cout << std::endl;
        }
        else
        {
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
                std::cout << "Some of the listed devices are not available or do not support double precision" << std::endl;
                std::exit(6);
            }
        }
    }

    const cl::Device* CResource_Manager::Get_Avaliable_Device()
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

    CArg_Parser::NRun_Type CResource_Manager::Get_Run_Type() const
    {
        return m_run_type;
    }
}