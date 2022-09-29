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

    int CFile_Stats::Run(config::TThread_Params* thread_config)
    {
        CFile_Stats_1 file_stats_1(m_file, m_is_valid_number);
        if (0 != file_stats_1.Run(thread_config))
        {
            return 1;
        }
        Copy_Values_From_File_Stats_1(file_stats_1);

        CFile_Stats_2 file_stats_2(m_file, m_is_valid_number, file_stats_1.Get_Values());
        if (0 != file_stats_2.Run(thread_config))
        {
            return 1;
        }
        Copy_Values_From_File_Stats_2(file_stats_2);

        return 0;
    }

    void CFile_Stats::Copy_Values_From_File_Stats_1(CFile_Stats_1& file_stats_1)
    {
        m_values.min = file_stats_1.Get_Min();
        m_values.max = file_stats_1.Get_Max();
        m_values.count = file_stats_1.Get_Count();
        m_values.mean = file_stats_1.Get_Mean();
    }

    void CFile_Stats::Copy_Values_From_File_Stats_2(CFile_Stats_2& file_stats_2)
    {
        m_values.var = file_stats_2.Get_Var();
        m_values.sd = file_stats_2.Get_SD();
        m_values.histogram = file_stats_2.Get_Histogram();
    }

    typename CFile_Stats::TValues CFile_Stats::Get_Values() const noexcept
    {
        return m_values;
    }

    std::ostream& operator<<(std::ostream& out, const CFile_Stats::TValues& values)
    {
        out << "min   = " << std::setprecision(kiv_ppr::config::DOUBLE_PRECISION) << values.min << '\n';
        out << "max   = " << values.max << '\n';
        out << "mean  = " << values.mean << '\n';
        out << "count = " << values.count << '\n';
        out << "var   = " << values.var << '\n';
        out << "sd    = " << values.sd;
        return out;
    }
}