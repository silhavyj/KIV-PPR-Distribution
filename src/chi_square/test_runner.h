#pragma once

#include <vector>

#include "chi_square.h"
#include "../processing/file_stats.h"

namespace kiv_ppr
{
    /// \author Jakub Silhavy
    ///
    /// This class kicks off the Chi-Square goodness of fit test for all
    /// of the distributions at once. Once they all finish, it collects
    /// the results presents them to the user.
    class CTest_Runner
    {
    public:
        /// Creates an instance of the class.
        /// \param values Statistical values calculated from the input file (min, max, mean, ...).
        /// \param p_critical Critical P-value used to determine whether a test is accepted or not.
        explicit CTest_Runner(CFile_Stats::TValues values, double p_critical) noexcept;

        /// Default destructor.
        ~CTest_Runner() = default;

        /// Runs all tests.
        void Run();

    private:
        /// Executes the Chi-Square goodness of fit test for the normal distribution.
        /// \return Result of the test.
        [[nodiscard]] CChi_Square::TResult Run_Normal() const;

        /// Executes the Chi-Square goodness of fit test for the uniform distribution.
        /// \return Result of the test.
        [[nodiscard]] CChi_Square::TResult Run_Uniform() const;

        /// Executes the Chi-Square goodness of fit test for the exponential distribution.
        /// \return Result of the test.
        [[nodiscard]] CChi_Square::TResult Run_Exponential() const;

        /// Executes the Chi-Square goodness of fit test for the poisson distribution.
        /// \return Result of the test.
        [[nodiscard]] CChi_Square::TResult Run_Poisson() const;

        /// Prints out the results of all the tests.
        /// \param results Collection of all the results
        void Print_Results(std::vector<CChi_Square::TResult>& results);

        /// Prints out a reasoning (explanation) behind the best result of all the tests.
        /// \param result The best result out of all the results.
        static void Print_Result_Reasoning(const CChi_Square::TResult& result);

    private:
        CFile_Stats::TValues m_values; ///< Statistical values calculated from the input file (min, max, mean, ...).
        double m_p_critical;           ///< Critical P-value used to determine whether a test is accepted or not.
    };
}

// EOF