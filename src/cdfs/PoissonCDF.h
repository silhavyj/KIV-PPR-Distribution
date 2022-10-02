#pragma once

#include "Cdf.h"

namespace kiv_ppr
{
    class CPoisson_CDF : public CCDF
    {
    public:
        static constexpr int NUMBER_OF_ESTIMATED_PARAMS = 1;
        static constexpr const char* NAME = "Poisson";

    public:
        explicit CPoisson_CDF(double lambda);
        ~CPoisson_CDF() override = default;

        [[nodiscard]] double operator()(double x) const override;
        static double Factorial(long x);

    private:
        double m_lambda;
    };
}