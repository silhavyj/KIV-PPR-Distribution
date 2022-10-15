#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include <cxxopts/cxxopts.hpp>

namespace kiv_ppr
{
    class CArg_Parser
    {
    public:
        enum class NRun_Type : uint8_t
        {
            ALL,
            SMP,
            OPENCL_DEVS
        };

    public:
        CArg_Parser(int argc, char* argv[]);
        ~CArg_Parser() = default;

        void Parse();
        void Parse_Options();
        void Print_Help();

        [[nodiscard]] bool Help();
        [[nodiscard]] double Get_P_Critical();
        [[nodiscard]] const char* Get_Filename();
        [[nodiscard]] std::vector<const char*> Get_OpenCL_Devs();
        [[nodiscard]]const char* Get_Run_Type_Str();

    private:
        static constexpr const char* ALL_RUN_TYPE = "all";
        static constexpr const char* SMP_RUN_TYPE = "smp";

    private:
        int m_argc;
        char** m_argv;
        const char* m_filename = nullptr;
        NRun_Type m_run_type{};
        std::vector<const char*> m_opencl_devs;
        cxxopts::Options m_options;
        cxxopts::ParseResult m_args;
    };
}