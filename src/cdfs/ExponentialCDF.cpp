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
            throw std::runtime_error("Exponential distribution (CDF) - lambda must be > 0");
        }
    }

    double CExponential_CDF::operator()(long double x) const
    {
        return static_cast<double>(1 - std::exp(-m_lambda * x));
    }
}