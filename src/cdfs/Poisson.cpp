#include <cmath>
#include <stdexcept>

#include "Poisson.h"

namespace kiv_ppr
{
    CPoisson_CDF::CPoisson_CDF(double lambda)
        : m_lambda(lambda)
    {
        if (m_lambda <= 0)
        {
            throw std::runtime_error("Lambda must be > 0");
        }
    }

    double CPoisson_CDF::operator()(double x) const
    {
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
                double ln_n = i * std::log(m_lambda) - (i * std::log(i) - i + log_6th_tail + log_pi_div_2);
                n = std::pow(M_E, ln_n - m_lambda);
            }
            else
            {
                if (i > 170 || std::numeric_limits<double>::infinity() == std::pow(m_lambda, i))
                {
                    infinity_is_found = true;
                    double log_6th_tail = std::log(i * (1 + 4 * i * (1 + 2 * i))) / 6;
                    double ln_n = i * std::log(m_lambda) - (i * std::log(i) - i + log_6th_tail + log_pi_div_2);
                    n = std::pow(M_E, ln_n - m_lambda);
                }
                else
                {
                    n = e_lambda * std::pow(m_lambda, i) / Factorial(i);
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

    double CPoisson_CDF::Factorial(long x)
    {
        long count = x;
        double factorial = 1;
        while (count >= 1)
        {
            factorial = factorial * count;
            --count;
        }
        return factorial;
    }
}