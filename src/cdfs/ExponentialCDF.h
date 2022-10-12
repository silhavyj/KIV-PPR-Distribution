#pragma once

#include "Cdf.h"

namespace kiv_ppr
{
    class CExponential_CDF : public CCDF
    {
    public:
        static constexpr int NUMBER_OF_ESTIMATED_PARAMS = 1;
        static constexpr const char* NAME = "Exponential";

    public:
        explicit CExponential_CDF(double lambda);
        ~CExponential_CDF() override = default;

        [[nodiscard]] double operator()(long double x) const override;

    private:
        double m_lambda;
    };
}