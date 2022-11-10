#include "resource_guard.h"
#include "singleton.h"

namespace kiv_ppr
{
    CResource_Guard::CResource_Guard() noexcept
        : m_device(nullptr)
    {

    }

    void CResource_Guard::Set_Device(const cl::Device* device) noexcept
    {
        m_device = device;
    }

    CResource_Guard::~CResource_Guard() noexcept(false)
    {
        // Make sure that the device is not null.
        if (nullptr != m_device)
        {
            auto resource_manager = Singleton<CResource_Manager>::Get_Instance();
            if (nullptr == resource_manager)
            {
                std::cout << "Error: resource manager is NULL" << std::endl;
                std::exit(22);
            }

            // Tell the resource manage that the OpenCL device is now available.
            resource_manager->Release_Device(m_device);
        }
    }
}

// EOF