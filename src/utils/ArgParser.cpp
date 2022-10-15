#include <algorithm>
#include <stdexcept>

#include "../Config.h"
#include "ArgParser.h"

namespace kiv_ppr
{
    CArg_Parser::CArg_Parser(int argc, char* argv[])
        : m_argc(argc),
          m_argv(argv),
          m_options("./pprsolver <filename> <all | SMP | dev1 dev2 dev3 ...>", "KIV/PPR Semester project - "
                    "Classification of statistical distributions (Chi-Square Goodness of Fit Test)")
    {
        m_options.add_options()
            ("p,p_critical", "Critical p value used in the Chi-square test", cxxopts::value<double>()->default_value(std::to_string(config::DEFAULT_P_CRITICAL)))
            ("h,help", "Print out this help menu");
    }

    bool CArg_Parser::Help()
    {
        return m_args.count("help") > 0;
    }

    void CArg_Parser::Print_Help()
    {
        std::cout << m_options.help() << '\n';
    }

    double CArg_Parser::Get_P_Critical()
    {
        return m_args["p_critical"].as<double>();
    }

    std::vector<const char*> CArg_Parser::Get_OpenCL_Devs()
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
        if (run_type == ALL_RUN_TYPE)
        {
            m_run_type = NRun_Type::ALL;
        }
        else if (run_type == SMP_RUN_TYPE)
        {
            m_run_type = NRun_Type::SMP;
        }
        else
        {
            m_run_type = NRun_Type::OPENCL_DEVS;
            for (int i = 2; i < m_argc; ++i)
            {
                const std::string dev = m_argv[i];
                if (dev.length() > 1 && dev[0] == '-' && dev[1] == '-')
                {
                    continue;
                }
                else if (dev[0] == '-')
                {
                    i += 2;
                    continue;
                }
                m_opencl_devs.emplace_back(m_argv[i]);
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
            case CArg_Parser::NRun_Type::ALL:
                return "all";

            case CArg_Parser::NRun_Type::SMP:
                return "SMP";

            case CArg_Parser::NRun_Type::OPENCL_DEVS:
                return "OpenCL devs";

            default:
                return "Unknown";
        }
    }
}