#pragma once

#include <string>
#include <memory>
#include <iostream>

#include "../cdfs/cdf.h"
#include "../processing/histogram.h"

namespace kiv_ppr
{
    /// \author Jakub Silhavy
    ///
    /// This class carries out the Chi-Square goodness of fit test for 
    /// a given distribution (https://en.wikipedia.org/wiki/Chi-squared_test).
    /// The test either accepts or rejects the H0 (null) hypothesis.
    ///
    /// Inspiration: https://www.youtube.com/watch?v=ztxKJlX7etU,
    /// https://learn.microsoft.com/en-us/archive/msdn-magazine/2017/march/test-run-chi-squared-goodness-of-fit-using-csharp
    class CChi_Square
    {
    public:
        /// Result status of the test
        enum class NTResult_Status : uint8_t
        {
            Accepted, ///< H0 hypothesis is accepted
            Rejected  ///< H0 hypothesis is rejected
        };

        /// Results of the test (encapsulation of multiple different values
        /// that will be printed out to the screen as a result of the test).
        struct TResult
        {
            NTResult_Status status{}; ///< Status of the result.
            double chi_square{};      ///< Chi-Square value (error - summed up squared differences).
            double p_value{};         ///< Calculated p-value to determine the result status of the test.
            long long df{};           ///< Degrees of freedom of the distribution used in the test.
            std::string name{};       ///< Name of the test.

            /// Compares two results. This function is used when sorting test results
            /// of different distributions. They are primarily compared by their p-values.
            /// However, if the none of the tests is accepted, they are sorted by their Chi-Square
            /// values (minimal error).
            /// \param other Result which this result will be compared to.
            /// \return true if the first result is better, false otherwise.
            bool operator<(const TResult& other) const noexcept;
        };

    public:
        /// Creates an instance of the class.
        /// \param name Name of the test.
        /// \param alpha_critical Critical value of alpha that determines whether the test is accepted or not.
        /// \param histogram Histogram used to calculate the Chi-Square value (actual values).
        /// \param cdf Cumulative distribution function (CDF) of the tested distribution (expected values).
        CChi_Square(std::string name,
                    double alpha_critical,
                    std::shared_ptr<CHistogram> histogram,
                    std::shared_ptr<CCDF> cdf) noexcept;

        /// Default class destructor.
        ~CChi_Square() = default;

        /// Runs the test. 
        /// \param estimated_parameters Number of estimated parameters of the tested distribution.
        /// \return Result of the tests containing calculated values to be printed out to the screen. 
        [[nodiscard]] TResult Run(int estimated_parameters);

    public:
        /// Calculates an expected value based on the given distribution.
        /// \param x Current input value to the CDF function of the tested distribution.
        /// \param x_prev Previous input value to the CDF function of the tested distribution.
        /// \param first_interval flag if the x value falls into the first interval. If so, the prev_value is not used.
        /// \return Calculated expected value.
        [[nodiscard]] double Calculate_E(double x, double x_prev, bool first_interval) const;

    private:
        /// Calculates the p-value using the ACM 299 algorithm.
        /// \param x Chi-Square value (summed up differences).
        /// \param df Degrees of freedom (differs with every distribution). 
        /// \return Calculated p-value
        double Calculate_P_Value(double x, long long df) noexcept;

        /// Calculates the p-value under the normal curve from -inf to z.
        /// This method is a part of ACM 299 algorithm.
        /// \param z Upper boundary to which the p-value under the normal curve is calculated.
        /// \return p-value under the normal curve from -inf to z.
        double Gauss(double z) noexcept;

        /// Calculates Exp(x).
        /// This method is a part of ACM 299 algorithm. 
        /// \param x Input value.
        /// \return 0, if x < -40 (ACM update remark (8)), std::exp(x) otherwise.
        double Exp(double x) noexcept;

    private:
        std::string m_name;                      ///< Name of the test to be carried out.
        double m_alpha_critical;                 ///< Critical value of alpha that determines whether the test is accepted or not.
        std::shared_ptr<CHistogram> m_histogram; ///< Histogram used to calculate the Chi-Square value (actual values).
        std::shared_ptr<CCDF> m_cdf;             ///< Cumulative distribution function (CDF) of the tested distribution (expected values).
    };
}

// EOF