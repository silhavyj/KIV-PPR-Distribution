#include "resource_guard.h"
#include "singleton.h"

namespace kiv_ppr
{
    CResource_Guard::CResource_Guard()
        : m_device(nullptr)
    {

    }

    void CResource_Guard::Set_Device(const cl::Device* device)
    {
        m_device = device;
    }

    CResource_Guard::~CResource_Guard()
    {
        if (nullptr != m_device)
        {
            auto resource_manager = Singleton<CResource_Manager>::Get_Instance();
            resource_manager->Release_Device(m_device);
        }
    }
}