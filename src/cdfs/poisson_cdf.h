#pragma once

#include <vector>

#include "cdf.h"

namespace kiv_ppr
{
    /// \author Jakub Silhavy
    /// 
    /// This class implements the CDF function of the poisson distribution.
    /// https://en.wikipedia.org/wiki/Poisson_distribution
    class CPoisson_CDF : public CCDF
    {
    public:
        static constexpr int Number_Of_Estimated_Params = 1; ///< Number of estimated parameter of the distribution (lambda)
        static constexpr const char* Name = "Poisson";       ///< Name of the distribution

    public:
        /// Creates an instance of the class. 
        /// \param lambda Lambda parameter of the poisson distribution.
        explicit CPoisson_CDF(double lambda);

        /// Default destructor.
        ~CPoisson_CDF() override = default;

        /// Call operator of the class.
        /// \return CDF(x)
        [[nodiscard]] double operator()(double x) const override;


    private:
        /// Creates a look-up table of factorials from 0 to MAX_CALCULABLE_FACTORIAL. 
        void Calculate_Factorials() noexcept;

    private:
        static constexpr int MAX_CALCULABLE_FACTORIAL = 170; ///< Highest factorial that fits into a double (!170)

    private:
        double m_lambda;                  ///< Lambda value of the poisson distribution
        std::vector<double> m_factorials; ///< Look-up table holding factorials from 0 to MAX_CALCULABLE_FACTORIAL
    };
}

// EOF