#include <iostream>

#include "ResourceManager.h"

namespace kiv_ppr
{
	void CResource_Manager::Find_Available_GPUs()
	{
		std::vector<cl::Platform> platforms;
		cl::Platform::get(&platforms);
	
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
				std::string extensions = device.getInfo<CL_DEVICE_EXTENSIONS>();
				if (extensions.find("cl_khr_fp64") == std::string::npos &&
					extensions.find("cl_amd_fp64") == std::string::npos)
				{
					continue;
				}
				m_gpu_devices.emplace_back(NDevice_Status::AVAILABLE, device);
			}
		}
	}

	void CResource_Manager::Print_Available_GPUs() const
	{
		if (m_gpu_devices.empty())
		{
			std::cout << "No available GPU devices supporting double precision found on this machine\n";
			return;
		}
		std::cout << "Available GPU devices supporting double precision:\n";
		for (const auto& [status, device] : m_gpu_devices)
		{
			std::cout << "Name: " << device.getInfo<CL_DEVICE_NAME>() << "\n";
		}
		std::cout << "\n";
	}
}