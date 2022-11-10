#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <unordered_set>

#include "../cxxopts/cxxopts.h"

namespace kiv_ppr
{
    /// \author Jakub Silhavy
    ///
    /// This class takes and parses input parameters passed into
    /// the program from the terminal. It uses the following header-only 
    /// library to parse optional parameters https://github.com/jarro2783/cxxopts.
    class CArg_Parser
    {
    public:
        /// Modes in which the program can be executed.
        enum class NRun_Type : uint8_t
        {
            All,        ///< SMP + all OpenCL devices
            SMP,        ///< Symmetric Multiprocessor
            OpenCL_Devs ///< OpenCL devices
        };

    public:
        /// Creates an instance of the class.
        /// \param argc Number of input parameters
        /// \param argv Input parameters
        explicit CArg_Parser(int argc, char* argv[]);

        /// Default destructor.
        ~CArg_Parser() = default;

        /// Parses compulsory parameters. These are the filename 
        /// and the mode in which the program should be executed.
        void Parse();

        /// Parses optional parameters. These, for instance, the
        /// critical P-value, data block size, watchdog period, etc. 
        void Parse_Options();

        /// Prints out help.
        void Print_Help();

        /// Return whether the use entered (--help or -h).
        /// \return true, if the user wishes to print out help, false otherwise.
        [[nodiscard]] bool Help();

        /// Returns the critical P-value.
        /// \return Critical P-Value.
        [[nodiscard]] double Get_P_Critical();

        /// Returns the size of a data block read from the input file.
        /// \return Size of a data block.
        [[nodiscard]] uint32_t Get_Block_Size_Per_Read();

        /// Returns number of seconds for which the watchdog will sleep (period).
        /// \return Watchdog period.
        [[nodiscard]] uint32_t Get_Watchdog_Sleep_Sec();

        /// Returns the number of threads the program will use.
        /// \return Number of thread used when processing the input file.
        [[nodiscard]] uint32_t Get_Number_Of_Threads();

        /// Returns the path to the input file.
        /// \return Path to the input file.
        [[nodiscard]] const char* Get_Filename() noexcept;

        /// Returns a set of OpenCL devices the user entered into the program.
        /// \return Entered OpenCL devices.
        [[nodiscard]] std::unordered_set<std::string> Get_OpenCL_Devs();

        /// Returns a text representation of the mode of the program (all, smp, ...)
        /// \return Text representation of the mode.
        [[nodiscard]] const char* Get_Run_Type_Str() noexcept;

        /// Returns the mode of the program (all, smp, ...)
        /// \return Mode of the program.
        [[nodiscard]] NRun_Type Get_Run_Type() noexcept;

    private:
        static constexpr const char* All_Run_Type_Str = "all"; ///< Text presentation of the 'all' mode
        static constexpr const char* SMP_Run_Type_Str = "smp"; ///< Text presentation of the 'smp' mode

    private:
        int m_argc;                                    ///< Total number of input arguments
        char** m_argv;                                 ///< Input arguments
        const char* m_filename = nullptr;              ///< Path to the input file
        NRun_Type m_run_type{};                        ///< Mode of the program (smp, all, ...)
        std::unordered_set<std::string> m_opencl_devs; ///< OpenCL devices the user wishes to use
        cxxopts::Options m_options;                    ///< Options of the program (-p, -w, ...)
        cxxopts::ParseResult m_args;                   ///< Argument parser
    };
}

// EOF