#pragma once

#include "cdf.h"

namespace kiv_ppr
{
    /// \author Jakub Silhavy
    /// 
    /// This class implements the CDF function of the normal distribution.
    /// https://en.wikipedia.org/wiki/Normal_distribution
    class CNormal_CDF : public CCDF
    {
    public:
        static constexpr int Number_Of_Estimated_Params = 2; ///< Number of estimated parameter of the distribution (mean, variance)
        static constexpr const char* Name = "Normal";        ///< Name of the distribution

    public:
        /// Creates an instance of the class. 
        /// \param mean Mean of the normal distribution.
        /// \param variance Variance of the normal distribution.
        explicit CNormal_CDF(double mean, double variance);

        /// Default destructor.
        ~CNormal_CDF() override = default;

        /// Call operator of the class.
        /// \return CDF(x)
        [[nodiscard]] double operator()(double x) const override;

    private:
        double m_mean;     ///< Mean of the normal distribution
        double m_variance; ///< Variance of the normal distribution
        double m_sd;       ///< Standard deviation of the normal distribution (std::sqrt(m_variance))
    };
}

// EOF