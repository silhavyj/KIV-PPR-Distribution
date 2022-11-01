#define _USE_MATH_DEFINES
#include <cmath>
#include <stdexcept>

#include "poisson_cdf.h"

namespace kiv_ppr
{
    CPoisson_CDF::CPoisson_CDF(double lambda)
        : m_lambda(lambda),
          m_factorials(MAX_CALCULABLE_FACTORIAL + 1, 1)
    {
        if (m_lambda <= 0)
        {
            throw std::runtime_error("Poisson distribution (CDF) - lambda must be > 0");
        }
        Calculate_Factorials();
    }

    void CPoisson_CDF::Calculate_Factorials()
    {
        for (size_t i = 1; i < m_factorials.size(); ++i)
        {
            m_factorials[i] = m_factorials[i - 1] * static_cast<double>(i);
        }
    }

    double CPoisson_CDF::operator()(double x) const
    {
        // https://www.codeproject.com/Tips/1216237/Csharp-Poisson-Cumulative-Distribution-for-large-L
        auto k = static_cast<long>(x);
        long i = 0;
        double sum = 0.0;
        bool infinity_is_found = false;
        double e_lambda = std::pow(M_E, -m_lambda);
        double log_pi_div_2 = std::log(M_PI) / 2.0;

        while (i <= k)
        {
            double n;
            if (infinity_is_found)
            {
                double log_6th_tail = std::log(i * (1 + 4 * i * (1 + 2 * i))) / 6;
                double ln_n = (double)i * std::log(m_lambda) - ((double)i * std::log(i) - (double)i + log_6th_tail + log_pi_div_2);
                n = std::pow(M_E, ln_n - m_lambda);
            }
            else
            {
                if (i > MAX_CALCULABLE_FACTORIAL || std::numeric_limits<double>::infinity() == std::pow(m_lambda, i))
                {
                    infinity_is_found = true;
                    double log_6th_tail = std::log(i * (1 + 4 * i * (1 + 2 * i))) / 6;
                    double ln_n = (double)i * std::log(m_lambda) - ((double)i * std::log(i) - (double)i + log_6th_tail + log_pi_div_2);
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