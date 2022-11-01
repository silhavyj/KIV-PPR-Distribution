#pragma once

#include <vector>

#include "chi_square.h"
#include "../processing/file_stats.h"

namespace kiv_ppr
{
    class CTest_Runner
    {
    public:
        explicit CTest_Runner(CFile_Stats::TValues values, double p_critical);
        ~CTest_Runner() = default;

        void Run();

    private:
        [[nodiscard]] CChi_Square::TResult Run_Normal() const;
        [[nodiscard]] CChi_Square::TResult Run_Uniform() const;
        [[nodiscard]] CChi_Square::TResult Run_Exponential() const;
        [[nodiscard]] CChi_Square::TResult Run_Poisson() const;

        void Print_Results(std::vector<CChi_Square::TResult>& results);
        static void Print_Result_Reasoning(CChi_Square::TResult& result);

    private:
        CFile_Stats::TValues m_values;
        double m_p_critical;
    };
}