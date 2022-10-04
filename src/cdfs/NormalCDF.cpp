#define _USE_MATH_DEFINES
#include <cmath>
#include <stdexcept>

#include "NormalCDF.h"

namespace kiv_ppr
{
    CNormal_CDF::CNormal_CDF(double mean, double variance)
            : m_mean(mean),
              m_variance(variance),
              m_sd(std::sqrt(variance))
    {
        if (m_variance <= 0)
        {
            throw std::runtime_error("Normal distribution (CDF) - variance must be > 0");
        }
    }

    double CNormal_CDF::operator()(double x) const
    {
        return 0.5 * (1 + std::erf((x - m_mean) / (m_sd * M_SQRT2)));
    }
}