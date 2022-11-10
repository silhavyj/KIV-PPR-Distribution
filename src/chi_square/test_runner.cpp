#include <utility>
#include <vector>
#include <future>
#include <iostream>
#include <iomanip>

#include "test_runner.h"
#include "../cdfs/normal_cdf.h"
#include "../cdfs/uniform_cdf.h"
#include "../cdfs/exponential_cdf.h"
#include "../cdfs/poisson_cdf.h"

namespace kiv_ppr
{
    CTest_Runner::CTest_Runner(CFile_Stats::TValues values, double p_critical) noexcept
        : m_values(std::move(values)),
          m_p_critical(p_critical)
    {

    }

    void CTest_Runner::Run()
    {
        // Container for all the tests.
        // The tests are executed in parallel.
        std::vector<std::future<kiv_ppr::CChi_Square::TResult>> workers;

        // Add normal and uniform distribution tests (they are executed always
        // regardless of the input numbers are).
        workers.push_back(std::async(std::launch::async, &CTest_Runner::Run_Normal, this));
        workers.push_back(std::async(std::launch::async, &CTest_Runner::Run_Uniform, this));

        // If the minimum is < 0, we can tell with certainty that the
        // data does come from the exponential or poisson distribution.
        if (m_values.first_iteration.min >= 0)
        {
            // Add exponential distribution test.
            workers.push_back(std::async(std::launch::async, &CTest_Runner::Run_Exponential, this));

            // If the data is not made up only of integers, they do not come from
            // the poisson distribution.
            if (m_values.first_iteration.all_ints)
            {
                workers.push_back(std::async(std::launch::async, &CTest_Runner::Run_Poisson, this));
            }
        }
     
        // Container for all the results.
        std::vector<kiv_ppr::CChi_Square::TResult> results(workers.size());
        try
        {
            // Perform all the tests and wait for them to finish.
            for (size_t i = 0; i < workers.size(); ++i)
            {
                results.at(i) = workers.at(i).get();
            }
        }
        catch (const std::exception& e)
        {
            // The tests throw an exception when an invalid value of one its parameters is provided (e.g. var < 0).
            std::cout << "Error while running Chi-Square test: " << e.what() << std::endl;
            std::exit(15);
        }

        // Sort the results (the final answer is at the first position).
        std::sort(results.begin(), results.end());

        // Print the results out to the screen.
        Print_Results(results);
    }

    void CTest_Runner::Print_Results(std::vector<CChi_Square::TResult>& results)
    {
        std::cout << std::setprecision(config::Double_Precision);
        std::cout << std::left << std::setw(15) << "Distribution"
                  << std::left << std::setw(15) << "Chi Square"
                  << std::left << std::setw(10) << "DF"
                  << std::left << std::setw(12) << "P-value"
                  << std::left << std::setw(15) << "Accepted" << std::endl;

        std::cout << std::left << std::setw(15) << "------------"
                  << std::left << std::setw(15) << "----------"
                  << std::left << std::setw(10) << "--"
                  << std::left << std::setw(12) << "-------"
                  << std::left << std::setw(15) << "--------" << std::endl;

        // Print out the results into a formatted table.
        for (const auto& result : results)
        {
            std::cout << std::left << std::setw(15) << result.name
                      << std::left << std::setw(15) << result.chi_square
                      << std::left << std::setw(10) << result.df
                      << std::left << std::setw(12) << result.p_value
                      << std::left << std::setw(15);

            switch (result.status)
            {
                case CChi_Square::NTResult_Status::Accepted:
                    std::cout << "YES";
                    break;

                case CChi_Square::NTResult_Status::Rejected:
                    std::cout << "NO";
                    break;

                default:
                    std::cout << "?";
            }
            std::cout << std::endl;
        }
        std::cout << std::left << std::setw(15) << "------------------------------------------------------------" << std::endl;

        // Print out the level of significance used for the tests.
        std::cout << "Level of significance = " << (m_p_critical * 100.0) << "%\n" << std::endl;

        // If the best result was rejected.
        if (results.begin()->status == CChi_Square::NTResult_Status::Rejected)
        {
            std::cout << "Statistically, none of the tests has been accepted.\n" << std::endl;

            // Print out the reasoning. For example, even though all tests were rejected, the data 
            // may corelate the most to the distribution with the best result (e.g. smallest Chi-Square value).
            Print_Result_Reasoning(*results.begin());
        }
        else if ((results.begin() + 1)->status == CChi_Square::NTResult_Status::Accepted)
        {
            // Multiple tests were accepted. Print out the reasoning 
            // (e.g. the result with the highest p-value is the winner).
            std::cout << "There are at least two tests that have been accepted.\n" << std::endl;
            Print_Result_Reasoning(*results.begin());
        }
        else
        {
            // Only one test was accept (the ideal situation).
            std::cout << "The date correlates the most to the " << results.begin()->name << " distribution." << std::endl;
        }
    }

    inline void CTest_Runner::Print_Result_Reasoning(const CChi_Square::TResult& result)
    {
        // If the DoF of the first test is < 0, it indicates that not enough data may have been provided.
        if (result.df < 0)
        {
            std::cout << "Judging by all degrees of freedom being less than 0, you may need to input more data into the program." << std::endl;
        }
        else
        {
            std::cout << "However, based on the Chi-Square error (" << result.chi_square
                      << ") and the degrees of freedom (" << result.df << "), the data seems to correlate the most to the " << result.name
                      << " distribution though it is STRONGLY recommended to double verify the answer." << std::endl;
        }
    }

    inline CChi_Square::TResult CTest_Runner::Run_Normal() const
    {
        // Create the test (normal distribution).
        kiv_ppr::CChi_Square chi_square_normal(
            CNormal_CDF::Name,
            m_p_critical,
            m_values.second_iteration.histogram,
            std::make_shared<kiv_ppr::CNormal_CDF>(m_values.first_iteration.mean, m_values.second_iteration.var)
        );

        // Run the test and return the result.
        return chi_square_normal.Run(CNormal_CDF::Number_Of_Estimated_Params);
    }

    inline CChi_Square::TResult CTest_Runner::Run_Uniform() const
    {
        // Create the test (uniform distribution).
        kiv_ppr::CChi_Square chi_square_uniform(
            CUniform_CDF::Name,
            m_p_critical,
            m_values.second_iteration.histogram,
            std::make_shared<kiv_ppr::CUniform_CDF>(m_values.first_iteration.min, m_values.first_iteration.max)
        );

        // Run the test and return the result.
        return chi_square_uniform.Run(CUniform_CDF::Number_Of_Estimated_Params);
    }

    inline CChi_Square::TResult CTest_Runner::Run_Exponential() const
    {
        // Create the test (exponential distribution).
        kiv_ppr::CChi_Square chi_square_exponential(
            CExponential_CDF::Name,
            m_p_critical,
            m_values.second_iteration.histogram,
            std::make_shared<kiv_ppr::CExponential_CDF>(1.0 / m_values.first_iteration.mean)
        );

        // Run the test and return the result.
        return chi_square_exponential.Run(CExponential_CDF::Number_Of_Estimated_Params);
    }

    inline CChi_Square::TResult CTest_Runner::Run_Poisson() const
    {
        // Create the test (poisson distribution).
        kiv_ppr::CChi_Square chi_square_poisson(
            CPoisson_CDF::Name,
            m_p_critical,
            m_values.second_iteration.histogram,
            std::make_shared<kiv_ppr::CPoisson_CDF>(m_values.first_iteration.mean)
        );

        // Run the test and return the result.
        return chi_square_poisson.Run(CPoisson_CDF::Number_Of_Estimated_Params);
    }
}

// EOF