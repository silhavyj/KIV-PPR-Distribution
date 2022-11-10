#pragma once

#include "cdf.h"

namespace kiv_ppr
{
    /// \author Jakub Silhavy
    /// 
    /// This class implements the CDF function of the uniform distribution.
    /// https://en.wikipedia.org/wiki/Continuous_uniform_distribution
    class CUniform_CDF : public CCDF
    {
    public:
        static constexpr int Number_Of_Estimated_Params = 2; ///< Number of estimated parameter of the distribution (a, b)
        static constexpr const char* Name = "Uniform";       ///< Name of the distribution

    public:
        /// Creates an instance of the class. 
        /// \param a a parameter of the uniform distribution (min).
        /// \param a b parameter of the uniform distribution (max).
        explicit CUniform_CDF(double a, double b);

        /// Default destructor.
        ~CUniform_CDF() override = default;

        /// Call operator of the class.
        /// \return CDF(x)
        [[nodiscard]] double operator()(double x) const noexcept override;

    private:
        double m_a; ///< a parameter of the uniform distribution (min).
        double m_b; ///< b parameter of the uniform distribution (max).
    };
}

// EOF