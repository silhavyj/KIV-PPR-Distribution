#pragma once

#include <memory>
#include <iostream>

#include "FirstIteration.h"
#include "SecondIteration.h"
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
            CFirst_Iteration::TValues first_iteration_values;
            CSecond_Iteration::TValues second_iteration_values;

            friend std::ostream& operator<<(std::ostream& out, const TValues& values);
        };

    public:
        explicit CFile_Stats(CFile_Reader<double>* file,
                             std::function<bool(double)> is_valid_number);

        ~CFile_Stats() = default;

        [[nodiscard]] TValues Get_Values() const noexcept;
        [[nodiscard]] int Process(config::TThread_Params* thread_config);

    private:
        CFile_Reader<double>* m_file;
        std::function<bool(double)> m_is_valid_number;
        TValues m_values;
    };
}