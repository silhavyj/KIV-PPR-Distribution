#pragma once

#include "resource_manager.h"

// https://stackoverflow.com/questions/56858213/how-to-create-nvidia-opencl-project/57017982#57017982
#define CL_USE_DEPRECATED_OPENCL_2_0_APIS
#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>

namespace kiv_ppr
{
    class CResource_Guard
    {
    public:      
        CResource_Guard();
        ~CResource_Guard();

        void Set_Device(const cl::Device* device);

    private:
        const cl::Device* m_device;
    };
}