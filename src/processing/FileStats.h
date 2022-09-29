#pragma once

#include <memory>
#include <iostream>

#include "FileStats1.h"
#include "FileStats2.h"
#include "../FileReader.h"
#include "../Histogram.h"
#include "../Config.h"

namespace kiv_ppr
{
    class CFile_Stats
    {
    public:
        struct TValues
        {
            double min;
            double max;
            size_t count;
            double mean;
            double var;
            double sd;
            std::shared_ptr<CHistogram> histogram;

            friend std::ostream& operator<<(std::ostream& out, const TValues& values);
        };

    public:
        explicit CFile_Stats(CFile_Reader<double>* file,
                             std::function<bool(double)> is_valid_number);

        ~CFile_Stats() = default;

        [[nodiscard]] TValues Get_Values() const noexcept;
        [[nodiscard]] int Process(config::TThread_Params* thread_config);

    private:
        void Copy_Values_From_File_Stats_1(CFile_Stats_1& file_stats_1);
        void Copy_Values_From_File_Stats_2(CFile_Stats_2& file_stats_2);

    private:
        CFile_Reader<double>* m_file;
        std::function<bool(double)> m_is_valid_number;
        TValues m_values;
    };
}