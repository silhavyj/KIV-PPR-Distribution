#include <utility>
#include <vector>
#include <future>
#include <iostream>

#include "TestRunner.h"
#include "../cdfs/NormalCDF.h"
#include "../cdfs/UniformCDF.h"
#include "../cdfs/ExponentialCDF.h"
#include "../cdfs/PoissonCDF.h"

namespace kiv_ppr
{
    CTest_Runner::CTest_Runner(CFile_Stats::TValues values)
        : m_values(std::move(values))
    {

    }

    void CTest_Runner::Run()
    {
        std::cout << "Running Chi Square statistical test...\n";
        std::vector<std::future<kiv_ppr::CChi_Square::TResult>> workers;

        workers.push_back(std::async(std::launch::async, &CTest_Runner::Run_Normal, this));
        workers.push_back(std::async(std::launch::async, &CTest_Runner::Run_Uniform, this));
        if (m_values.first_iteration.min >= 0)
        {
            workers.push_back(std::async(std::launch::async, &CTest_Runner::Run_Exponential, this));
            if (m_values.first_iteration.normal_numbers)
            {
                workers.push_back(std::async(std::launch::async, &CTest_Runner::Run_Poisson, this));
            }
        }
        std::vector<kiv_ppr::CChi_Square::TResult> results(workers.size());
        for (size_t i = 0; i < workers.size(); ++i)
        {
            results[i] = workers[i].get();
            std::cout << results[i] << '\n';
        }
        std::sort(results.begin(), results.end());
        std::cout << "Result: " << results.begin()->name;
    }

    CChi_Square::TResult CTest_Runner::Run_Normal() const
    {
        kiv_ppr::CChi_Square chi_square_normal(
            CNormal_CDF::NAME,
            m_values.second_iteration.histogram,
            std::make_shared<kiv_ppr::CNormal_CDF>(m_values.first_iteration.mean, m_values.second_iteration.var)
        );
        return chi_square_normal.Run(CNormal_CDF::NUMBER_OF_ESTIMATED_PARAMS);
    }

    CChi_Square::TResult CTest_Runner::Run_Uniform() const
    {
        kiv_ppr::CChi_Square chi_square_uniform(
            CUniform_CDF::NAME,
            m_values.second_iteration.histogram,
            std::make_shared<kiv_ppr::CUniform_CDF>(m_values.first_iteration.min, m_values.first_iteration.max)
        );
        return chi_square_uniform.Run(CUniform_CDF::NUMBER_OF_ESTIMATED_PARAMS);
    }

    CChi_Square::TResult CTest_Runner::Run_Exponential() const
    {
        kiv_ppr::CChi_Square chi_square_exponential(
            CExponential_CDF::NAME,
            m_values.second_iteration.histogram,
            std::make_shared<kiv_ppr::CExponential_CDF>(1.0 / m_values.first_iteration.mean)
        );
        return chi_square_exponential.Run(CExponential_CDF::NUMBER_OF_ESTIMATED_PARAMS);
    }

    CChi_Square::TResult CTest_Runner::Run_Poisson() const
    {
        kiv_ppr::CChi_Square chi_square_poisson(
            CPoisson_CDF::NAME,
            m_values.second_iteration.histogram,
            std::make_shared<kiv_ppr::CPoisson_CDF>(m_values.first_iteration.mean)
        );
        return chi_square_poisson.Run(CPoisson_CDF::NUMBER_OF_ESTIMATED_PARAMS);
    }
}