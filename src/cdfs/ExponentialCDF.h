#pragma once

#include "Cdf.h"

namespace kiv_ppr
{
    class CExponential_CDF : public CCDF
    {
    public:
        static constexpr int Number_Of_Estimated_Params = 1;
        static constexpr const char* Name = "Exponential";

    public:
        explicit CExponential_CDF(double lambda);
        ~CExponential_CDF() override = default;

        [[nodiscard]] double operator()(double x) const override;

    private:
        double m_lambda;
    };
}