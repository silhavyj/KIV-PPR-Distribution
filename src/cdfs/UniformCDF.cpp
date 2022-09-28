#include <stdexcept>

#include "UniformCDF.h"

namespace kiv_ppr
{
    CUniform_CDF::CUniform_CDF(double a, double b)
            : m_a(a),
              m_b(b)
    {
        if (m_a >= m_b)
        {
            throw std::runtime_error("a is greater than b");
        }
    }

    [[nodiscard]] double CUniform_CDF::operator()(double x) const
    {
        if (x < m_a)
        {
            return 0;
        }
        else if (x > m_b)
        {
            return 1;
        }
        return (x - m_a) / (m_b - m_a);
    }
}