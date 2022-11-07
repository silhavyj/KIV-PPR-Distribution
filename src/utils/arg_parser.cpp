#include <algorithm>
#include <stdexcept>

#include "../config.h"
#include "arg_parser.h"

namespace kiv_ppr
{
    CArg_Parser::CArg_Parser(int argc, char* argv[])
        : m_argc(argc),
          m_argv(argv),
          m_options("pprsolver.exe <filename> <all | SMP | \"dev1\" \"dev2\" \"dev3\" ...>", "KIV/PPR Semester project - "
                    "Classification of statistical distributions (Chi-Square Goodness of Fit Test)")
    {
        m_options.add_options()
            ("p,p_critical", "Critical p value used in the Chi-square test", cxxopts::value<double>()->default_value(std::to_string(config::chi_square::Default_P_Critical)))
            ("b,block_size", "Number of bytes read from the input file at a time (block size)", cxxopts::value<uint32_t>()->default_value(std::to_string(config::processing::Block_Size_Per_Read)))
            ("w,watchdog_period", "How often the watchdog checks if the program is working correctly [s]", cxxopts::value<uint32_t>()->default_value(std::to_string(config::processing::Watchdog_Sleep_Sec)))
            ("h,help", "Print out this help menu");
    }

    bool CArg_Parser::Help()
    {
        return m_args.count("help") > 0;
    }

    void CArg_Parser::Print_Help()
    {
        std::cout << m_options.help() << std::endl;
    }

    double CArg_Parser::Get_P_Critical()
    {
        return m_args["p_critical"].as<double>();
    }

    uint32_t CArg_Parser::Get_Block_Size_Per_Read()
    {
        return m_args["block_size"].as<uint32_t>() / sizeof(double);
    }

    uint32_t CArg_Parser::Get_Watchdog_Sleep_Sec()
    {
        return m_args["watchdog_period"].as<uint32_t>();
    }

    std::unordered_set<std::string> CArg_Parser::Get_OpenCL_Devs()
    {
        return m_opencl_devs;
    }

    const char* CArg_Parser::Get_Filename()
    {
        return m_filename;
    }

    void CArg_Parser::Parse_Options()
    {
        m_args = m_options.parse(m_argc, m_argv);
    }

    CArg_Parser::NRun_Type CArg_Parser::Get_Run_Type()
    {
        return m_run_type;
    }

    void CArg_Parser::Parse()
    {
        if (m_argc < 3)
        {
            throw std::invalid_argument{"Invalid number of parameters"};
        }
        m_filename = m_argv[1];
        std::string run_type = m_argv[2];

        std::transform(run_type.begin(), run_type.end(), run_type.begin(), [](unsigned char c) {
            return std::tolower(c);
        });
        if (run_type == All_Run_Type_Str)
        {
            m_run_type = NRun_Type::All;
        }
        else if (run_type == SMP_Run_Type_Str)
        {
            m_run_type = NRun_Type::SMP;
        }
        else
        {
            m_run_type = NRun_Type::OpenCL_Devs;
            for (int i = 2; i < m_argc; ++i)
            {
                const std::string dev = m_argv[i];
                if (dev.length() > 1 && dev[0] == '-' && dev[1] == '-')
                {
                    continue;
                }
                else if (dev[0] == '-')
                {
                    ++i;
                }
                else
                {
                    m_opencl_devs.insert(m_argv[i]);
                }
            }
            if (m_opencl_devs.empty())
            {
                throw std::invalid_argument{"No OpenCL devices provided"};
            }
        }
    }

    const char* CArg_Parser::Get_Run_Type_Str()
    {
        switch (m_run_type)
        {
            case CArg_Parser::NRun_Type::All:
                return "all";

            case CArg_Parser::NRun_Type::SMP:
                return "SMP";

            case CArg_Parser::NRun_Type::OpenCL_Devs:
                return "OpenCL devs";

            default:
                return "Unknown";
        }
    }
}