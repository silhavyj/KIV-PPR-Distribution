#include <cmath>
#include <stdexcept>

#include "ExponentialCDF.h"

namespace kiv_ppr
{
    CExponential_CDF::CExponential_CDF(double lambda)
            : m_lambda(lambda)
    {
        if (m_lambda <= 0)
        {
            throw std::runtime_error("Lambda must be > 0");
        }
    }

    [[nodiscard]] double CExponential_CDF::operator()(double x) const
    {
        return 1 - std::exp(-m_lambda * x);
    }
}