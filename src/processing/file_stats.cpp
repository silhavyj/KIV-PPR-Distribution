#include <iomanip>

#include "file_stats.h"

namespace kiv_ppr
{
    CFile_Stats::CFile_Stats(CFile_Reader<double>* file)
        : m_file(file),
          m_values{}
    {

    }

    int CFile_Stats::Process(config::TThread_Params* thread_config)
    {
        // Create an instance of the first filer iteration.
        CFirst_Iteration first_iteration(m_file);

        // Reade the input file (1).
        if (0 != first_iteration.Run(thread_config))
        {
            return 1;
        }

        // Store the values calculated in the first iteration.
        m_values.first_iteration = first_iteration.Get_Values();

        // Create an instance of the second file iteration.
        CSecond_Iteration second_iteration(m_file, &m_values.first_iteration);

        // Read the input file (2).
        if (0 != second_iteration.Run(thread_config))
        {
            return 1;
        }
        
        // Store the values calculated in the second iteration.
        m_values.second_iteration = second_iteration.Get_Values();

        return 0;
    }

    typename CFile_Stats::TValues CFile_Stats::Get_Values() const noexcept
    {
        return m_values;
    }

    std::ostream& operator<<(std::ostream& out, const CFile_Stats::TValues& values)
    {
        double scale_factor = values.first_iteration.min < 0 ? config::processing::Scale_Factor : 1;
        out << "min   = " << std::setprecision(kiv_ppr::config::Double_Precision) << (values.first_iteration.min * scale_factor) << '\n';
        out << "max   = " << (values.first_iteration.max * scale_factor)   << '\n';
        out << "mean  = " << (values.first_iteration.mean * scale_factor)  << '\n';
        out << "count = " << values.first_iteration.count << '\n';
        out << "var   = " << (values.second_iteration.var * scale_factor) << '\n';
        out << "sd    = " << (values.second_iteration.sd * scale_factor);
        return out;
    }
}