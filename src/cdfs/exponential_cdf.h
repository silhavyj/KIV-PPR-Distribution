#pragma once

#include "Cdf.h"

namespace kiv_ppr
{
    /// \author Jakub Silhavy
    /// 
    /// This class implements the CDF function of the exponential distribution.
    /// https://en.wikipedia.org/wiki/Exponential_distribution
    class CExponential_CDF : public CCDF
    {
    public:
        static constexpr int Number_Of_Estimated_Params = 1; ///< Number of estimated parameter of the distribution (lambda)
        static constexpr const char* Name = "Exponential";   ///< Name of the distribution

    public:
        /// Creates an instance of the class. 
        /// \param lambda Lambda parameter of the exponential distribution.
        explicit CExponential_CDF(double lambda);

        /// Default destructor.
        ~CExponential_CDF() override = default;

        /// Call operator of the class.
        /// \return CDF(x)
        [[nodiscard]] double operator()(double x) const noexcept override;

    private:
        double m_lambda; ///< Lambda parameter of the distribution
    };
}