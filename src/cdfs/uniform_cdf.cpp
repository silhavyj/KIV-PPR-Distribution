#include <stdexcept>

#include "uniform_cdf.h"

namespace kiv_ppr
{
    CUniform_CDF::CUniform_CDF(double a, double b)
        : m_a(a),
          m_b(b)
    {
        // Make sure that a parameter (min) is less than b parameter (max).
        if (m_a >= m_b)
        {
            throw std::runtime_error("Uniform distribution (CDF) - min is greater than max");
        }
    }

    double CUniform_CDF::operator()(double x) const
    {
        if (x < m_a)
        {
            return 0;
        }
        else if (x > m_b)
        {
            return 1;
        }
        return static_cast<double>((x - m_a) / (m_b - m_a));
    }
}

// EOF