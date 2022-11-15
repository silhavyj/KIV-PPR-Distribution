#pragma once

#include "resource_manager.h"
#include "../opencl.h"

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
        ~CResource_Guard() noexcept(false);

        /// Delete copy constructor.
        CResource_Guard(const CResource_Guard&) = delete;

        /// Delete copy constructor (R value).
        CResource_Guard(CResource_Guard&&) = delete;

        /// Delete assignment operator.
        CResource_Guard& operator=(const CResource_Guard&) = delete;

        /// Delete assignment operator (R value).
        CResource_Guard& operator=(const CResource_Guard&&) = delete;

        /// Sets the device the class will guard.  
        /// \param device OpenCL device that will released 
        void Set_Device(const cl::Device* device) noexcept;

    private:
        const cl::Device* m_device; ///< OpenCL device
    };
}

// EOF