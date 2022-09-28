#pragma once

#include "Cdf.h"

namespace kiv_ppr
{
    class CUniform_CDF : public CCDF
    {
    public:
        explicit CUniform_CDF(double a, double b);
        ~CUniform_CDF() override = default;

        [[nodiscard]] double operator()(double x) const override;

    private:
        double m_a;
        double m_b;
    };
}