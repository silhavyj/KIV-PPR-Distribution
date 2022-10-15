#pragma once

#include <vector>

#define CL_USE_DEPRECATED_OPENCL_2_0_APIS
#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_TARGET_OPENCL_VERSION 200
#include <CL/opencl.hpp>

namespace kiv_ppr
{
	class CResource_Manager
	{
	public:
		void Find_Available_GPUs();
		void Print_Available_GPUs() const;
		bool All_Devices_Available(std::vector<std::string>& devices);

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
	};
}