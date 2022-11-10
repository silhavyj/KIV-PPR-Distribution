// Required by MSVC when using constants such as M_E or M_PI.
#define _USE_MATH_DEFINES

#include <cmath>
#include <stdexcept>
#include <limits>

#include "poisson_cdf.h"

namespace kiv_ppr
{
    CPoisson_CDF::CPoisson_CDF(double lambda)
        : m_lambda(lambda),
          m_factorials(MAX_CALCULABLE_FACTORIAL + 1, 1)
    {
        // Make sure that a valid value of the lambda parameter was provided.
        if (m_lambda <= 0)
        {
            throw std::runtime_error("Poisson distribution (CDF) - lambda must be > 0");
        }

        // Pre-calculate all factorials (0 to MAX_CALCULABLE_FACTORIAL).
        Calculate_Factorials();
    }

    void CPoisson_CDF::Calculate_Factorials() noexcept
    {
        for (size_t i = 1; i < m_factorials.size(); ++i)
        {
            m_factorials.at(i) = m_factorials.at(i - 1) * static_cast<double>(i);
        }
    }

    double CPoisson_CDF::operator()(double x) const
    {
        // This algorithm uses the Ramanujan's factorial approximation.
        // https://www.codeproject.com/Tips/1216237/Csharp-Poisson-Cumulative-Distribution-for-large-L

        const auto k = static_cast<long>(x);
        long i = 0;
        double sum = 0.0;
        bool infinity_is_found = false;
        const double e_lambda = std::pow(M_E, -m_lambda);
        const double log_pi_div_2 = std::log(M_PI) / 2.0;

        while (i <= k)
        {
            double n{};
            if (infinity_is_found)
            {
                const double log_6th_tail = std::log(i * (1 + 4 * i * (1 + 2 * i))) / 6;
                const double ln_n = static_cast<double>(i) * std::log(m_lambda) - (static_cast<double>(i) * std::log(i) - static_cast<double>(i) + log_6th_tail + log_pi_div_2);
                n = std::pow(M_E, ln_n - m_lambda);
            }
            else
            {
                if (i > MAX_CALCULABLE_FACTORIAL || std::numeric_limits<double>::infinity() == std::pow(m_lambda, i))
                {
                    infinity_is_found = true;
                    const double log_6th_tail = std::log(i * (1 + 4 * i * (1 + 2 * i))) / 6;
                    const double ln_n = static_cast<double>(i) * std::log(m_lambda) - (static_cast<double>(i) * std::log(i) - static_cast<double>(i) + log_6th_tail + log_pi_div_2);
                    n = std::pow(M_E, ln_n - m_lambda);
                }
                else
                {
                    n = e_lambda * std::pow(m_lambda, i) / m_factorials.at(i);
                }
            }
            sum += n;
            ++i;
        }
        if (sum > 1)
        {
            return 1.0;
        }
        return sum;
    }
}

// EOF