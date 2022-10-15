#include <utility>
#include <vector>
#include <future>
#include <iostream>
#include <iomanip>

#include "TestRunner.h"
#include "../cdfs/NormalCDF.h"
#include "../cdfs/UniformCDF.h"
#include "../cdfs/ExponentialCDF.h"
#include "../cdfs/PoissonCDF.h"

namespace kiv_ppr
{
    CTest_Runner::CTest_Runner(CFile_Stats::TValues values, double p_critical)
        : m_values(std::move(values)),
          m_p_critical(p_critical)
    {

    }

    void CTest_Runner::Run()
    {
        std::vector<std::future<kiv_ppr::CChi_Square::TResult>> workers;

        workers.push_back(std::async(std::launch::async, &CTest_Runner::Run_Normal, this));
        workers.push_back(std::async(std::launch::async, &CTest_Runner::Run_Uniform, this));
        if (m_values.first_iteration.min >= 0)
        {
            workers.push_back(std::async(std::launch::async, &CTest_Runner::Run_Exponential, this));
            if (m_values.first_iteration.all_ints)
            {
                workers.push_back(std::async(std::launch::async, &CTest_Runner::Run_Poisson, this));
            }
        }
        std::vector<kiv_ppr::CChi_Square::TResult> results(workers.size());
        for (size_t i = 0; i < workers.size(); ++i)
        {
            results[i] = workers[i].get();
        }
        std::sort(results.begin(), results.end());
        Print_Results(results);
    }

    void CTest_Runner::Print_Results(std::vector<CChi_Square::TResult>& results)
    {
        std::cout << std::setprecision(config::DOUBLE_PRECISION);
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

        for (const auto& result : results)
        {
            std::cout << std::left << std::setw(15) << result.name
                      << std::left << std::setw(15) << result.chi_square
                      << std::left << std::setw(10) << result.df
                      << std::left << std::setw(12) << result.p_value
                      << std::left << std::setw(15);

            switch (result.status)
            {
                case CChi_Square::ETResult_Status::ACCEPTED:
                    std::cout << "YES";
                    break;
                case CChi_Square::ETResult_Status::REJECTED:
                    std::cout << "NO";
                    break;
                default:
                    std::cout << "?";
            }
            std::cout << std::endl;
        }
        std::cout << std::left << std::setw(15) << "------------------------------------------------------------" << std::endl;
        std::cout << "Level of significance = " << (m_p_critical * 100.0) << "%\n\n";

        if (results.begin()->status == CChi_Square::ETResult_Status::REJECTED)
        {
            std::cout << "Statistically, none of the tests has been accepted.\n\n";
            Print_Result_Reasoning(*results.begin());
        }
        else if ((results.begin() + 1)->status == CChi_Square::ETResult_Status::ACCEPTED)
        {
            std::cout << "There are at least two tests that have been accepted.\n\n";
            Print_Result_Reasoning(*results.begin());
        }
        else
        {
            std::cout << "The date correlates the most to the " << results.begin()->name << " distribution.\n";
        }
    }

    inline void CTest_Runner::Print_Result_Reasoning(CChi_Square::TResult& result)
    {
        std::cout << "However, based on the Chi-Square error (" << result.chi_square
                  << ") and the degrees of freedom (" << result.df << "), the data seems to correlate the most to the " << result.name
                  << " distribution though it is STRONGLY recommended to double verify the answer.\n";
    }

    inline CChi_Square::TResult CTest_Runner::Run_Normal() const
    {
        kiv_ppr::CChi_Square chi_square_normal(
            CNormal_CDF::NAME,
            m_p_critical,
            m_values.second_iteration.histogram,
            std::make_shared<kiv_ppr::CNormal_CDF>(m_values.first_iteration.mean, m_values.second_iteration.var)
        );

        return chi_square_normal.Run(CNormal_CDF::NUMBER_OF_ESTIMATED_PARAMS);
    }

    inline CChi_Square::TResult CTest_Runner::Run_Uniform() const
    {
        kiv_ppr::CChi_Square chi_square_uniform(
            CUniform_CDF::NAME,
            m_p_critical,
            m_values.second_iteration.histogram,
            std::make_shared<kiv_ppr::CUniform_CDF>(m_values.first_iteration.min, m_values.first_iteration.max)
        );

        return chi_square_uniform.Run(CUniform_CDF::NUMBER_OF_ESTIMATED_PARAMS);
    }

    inline CChi_Square::TResult CTest_Runner::Run_Exponential() const
    {
        kiv_ppr::CChi_Square chi_square_exponential(
            CExponential_CDF::NAME,
            m_p_critical,
            m_values.second_iteration.histogram,
            std::make_shared<kiv_ppr::CExponential_CDF>(1.0 / m_values.first_iteration.mean)
        );

        return chi_square_exponential.Run(CExponential_CDF::NUMBER_OF_ESTIMATED_PARAMS);
    }

    inline CChi_Square::TResult CTest_Runner::Run_Poisson() const
    {
        kiv_ppr::CChi_Square chi_square_poisson(
            CPoisson_CDF::NAME,
            m_p_critical,
            m_values.second_iteration.histogram,
            std::make_shared<kiv_ppr::CPoisson_CDF>(m_values.first_iteration.mean)
        );

        return chi_square_poisson.Run(CPoisson_CDF::NUMBER_OF_ESTIMATED_PARAMS);
    }
}