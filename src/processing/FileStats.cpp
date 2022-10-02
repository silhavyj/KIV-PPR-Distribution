#include <iomanip>

#include "FileStats.h"

namespace kiv_ppr
{
    CFile_Stats::CFile_Stats(CFile_Reader<double>* file,
                             std::function<bool(double)> is_valid_number)
        : m_file(file),
          m_is_valid_number(std::move(is_valid_number)),
          m_values{}
    {

    }

    int CFile_Stats::Process(config::TThread_Params* thread_config)
    {
        CFirst_Iteration first_iteration(m_file, m_is_valid_number);
        if (0 != first_iteration.Run(thread_config))
        {
            return 1;
        }
        m_values.first_iteration = first_iteration.Get_Values();

        CSecond_Iteration second_iteration(m_file, m_is_valid_number, m_values.first_iteration);
        if (0 != second_iteration.Run(thread_config))
        {
            return 1;
        }
        m_values.second_iteration = second_iteration.Get_Values();

        return 0;
    }

    typename CFile_Stats::TValues CFile_Stats::Get_Values() const noexcept
    {
        return m_values;
    }

    std::ostream& operator<<(std::ostream& out, const CFile_Stats::TValues& values)
    {
        out << values.first_iteration << '\n';
        out << values.second_iteration;
        return out;
    }
}