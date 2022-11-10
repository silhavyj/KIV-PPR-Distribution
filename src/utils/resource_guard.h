#pragma once

#include "resource_manager.h"

#define CL_USE_DEPRECATED_OPENCL_2_0_APIS
#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>

namespace kiv_ppr
{
    /// \author Jakub Silhavy
    ///
    /// This class works as a wrapper around an OpenCL device.
    /// It only purpose is to release the device (mark it as available)
    /// once a worker thread finishes its work (leaves the scope).
    class CResource_Guard
    {
    public:
        /// Creates an instance of the class.      
        CResource_Guard() noexcept;

        /// Calls the resource manager and frees the device.
        ~CResource_Guard();

        /// Sets the device the class will guard.  
        /// \param device OpenCL device that will released 
        void Set_Device(const cl::Device* device) noexcept;

    private:
        const cl::Device* m_device; ///< OpenCL device
    };
}