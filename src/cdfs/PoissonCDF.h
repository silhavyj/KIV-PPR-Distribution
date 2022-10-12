#pragma once

#include <vector>

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
        void Calculate_Factorials();

    private:
        static constexpr int MAX_CALCULABLE_FACTORIAL = 170;

    private:
        double m_lambda;
        std::vector<double> m_factorials;
    };
}