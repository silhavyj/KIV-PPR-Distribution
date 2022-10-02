#pragma once

#include "ChiSquare.h"
#include "../processing/FileStats.h"

namespace kiv_ppr
{
    class CTest_Runner
    {
    public:
        explicit CTest_Runner(CFile_Stats::TValues values);
        ~CTest_Runner() = default;

        void Run();

    private:
        CChi_Square::TResult Run_Normal() const;
        CChi_Square::TResult Run_Uniform() const;
        CChi_Square::TResult Run_Exponential() const;
        CChi_Square::TResult Run_Poisson() const;

    private:
        CFile_Stats::TValues m_values;
    };
}