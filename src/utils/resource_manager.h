#pragma once

#include <vector>
#include <mutex>
#include <unordered_set>

#include "../opencl.h"
#include "arg_parser.h"

namespace kiv_ppr
{
    /// \author Jakub Silhavy
    ///
    /// The purpose of this class is to hand out OpenCL devices 
    /// to worker threads. This class is used as a singleton (see singleton.h).
    class CResource_Manager
    {
    public:
        /// Default constructor.
        CResource_Manager() = default;

        // Default destructor.
        ~CResource_Manager() = default;

        /// Sets the run mode of the application.
        /// The run mode affects what device will be available for the worker threads (all/some).
        /// \param run_type Run type (mode) of the program
        /// \param use_gpu_only Whether or not the program should only use GPUs as OpenCL devices.
        void Set_Run_Type(CArg_Parser::NRun_Type run_type, bool use_gpu_only) noexcept;

        /// Finds available OpenCL devices.
        /// \param listed_devices List of OpenCL devices the user entered into the program.
        void Find_Available_GPUs(const std::unordered_set<std::string>& listed_devices);

        /// Finds an available OpenCL device and returns it to the caller.
        /// \return Available OpenCL device, if it exists. Otherwise, nullptr.
        [[nodiscard]] const cl::Device* Get_Available_Device();

        /// Releases an OpenCL device.
        /// This method is called when the worker thread does not need the device anymore.
        /// \param device OpenCL device to be released (marked as available again).
        void Release_Device(const cl::Device* device);
        
        /// Returns the run type of the program (mode - all, smp, ...)
        /// \return Mode of the program
        [[nodiscard]] CArg_Parser::NRun_Type Get_Run_Type() const noexcept;

    private:
        /// Prints out found OpenCL devices. It also checks that all of the listed
        /// devices have been found on the machine.
        /// \param found_devices OpenCL devices found on the machine
        /// \param listed_devices OpenCL devices listed by the user
        void Print_Found_Devs(const std::unordered_set<std::string>& found_devices,
                              const std::unordered_set<std::string>& listed_devices);

    private:
        /// Status of an OpenCL device
        enum class NDevice_Status : uint8_t
        {
            Available, ///< Free (ready to be used)
            Taken      ///< Being used by a worker thread
        };

        /// Resource (OpenCL device + its status)
        struct TResource
        {
            NDevice_Status m_status; ///< Status of the OpenCL device
            cl::Device m_device;     ///< OpenCL device

            /// Creates an instance of the struct 
            /// \param status Status of the OpenCL device
            /// \param device OpenCL device
            TResource(NDevice_Status status, cl::Device device);
        };

    private:
        std::vector<TResource> m_gpu_devices; ///< Available resources (OpenCL devices)
        CArg_Parser::NRun_Type m_run_type{};  ///< Mode of the program (all, smp, ...)
        std::mutex m_mtx;                     ///< Mutex used when a device is being allocated/released
        bool m_use_gpu_only = false;          ///< Whether or not the program should use only GPUs as OpenCL devices.
    };
}

// EOF