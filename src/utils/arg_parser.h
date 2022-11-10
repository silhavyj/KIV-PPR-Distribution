#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <unordered_set>

#include "../cxxopts/cxxopts.hpp"

namespace kiv_ppr
{
    class CArg_Parser
    {
    public:
        enum class NRun_Type : uint8_t
        {
            All,
            SMP,
            OpenCL_Devs
        };

    public:
        CArg_Parser(int argc, char* argv[]);
        ~CArg_Parser() = default;

        void Parse();
        void Parse_Options();
        void Print_Help();

        [[nodiscard]] bool Help();
        [[nodiscard]] double Get_P_Critical();
        [[nodiscard]] uint32_t Get_Block_Size_Per_Read();
        [[nodiscard]] uint32_t Get_Watchdog_Sleep_Sec();
        [[nodiscard]] uint32_t Get_Number_Of_Threads();
        [[nodiscard]] const char* Get_Filename() noexcept;
        [[nodiscard]] std::unordered_set<std::string> Get_OpenCL_Devs();
        [[nodiscard]] const char* Get_Run_Type_Str() noexcept;
        [[nodiscard]] NRun_Type Get_Run_Type() noexcept;

    private:
        static constexpr const char* All_Run_Type_Str = "all";
        static constexpr const char* SMP_Run_Type_Str = "smp";

    private:
        int m_argc;
        char** m_argv;
        const char* m_filename = nullptr;
        NRun_Type m_run_type{};
        std::unordered_set<std::string> m_opencl_devs;
        cxxopts::Options m_options;
        cxxopts::ParseResult m_args;
    };
}