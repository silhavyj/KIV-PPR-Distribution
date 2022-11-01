#pragma once

#include "cdf.h"

namespace kiv_ppr
{
    class CNormal_CDF : public CCDF
    {
    public:
        static constexpr int Number_Of_Estimated_Params = 2;
        static constexpr const char* Name = "Normal";

    public:
        explicit CNormal_CDF(double mean, double variance);
        ~CNormal_CDF() override = default;

        [[nodiscard]] double operator()(double x) const override;

    private:
        double m_mean;
        double m_variance;
        double m_sd;
    };
}