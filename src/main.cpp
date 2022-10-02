#include <iostream>
#include <thread>
#include <future>
#include <vector>

#include "utils/Utils.h"
#include "Config.h"
#include "FileReader.h"
#include "processing/FileStats.h"
#include "cdfs/Poisson.h"
#include "cdfs/NormalCDF.h"
#include "cdfs/UniformCDF.h"
#include "cdfs/ExponentialCDF.h"
#include "tests/ChiSquare.h"

static std::string filename{"../../data/gauss"};

void Run_Statistical_Tests(kiv_ppr::CFile_Stats::TValues& values)
{
    std::cout << "Running Chi Square statistical test...\n";
    std::vector<std::future<kiv_ppr::CChi_Square::TResult>> workers;

    workers.push_back(std::async(std::launch::async, [&]() {
        kiv_ppr::CChi_Square chi_square_normal(
            "Normal",
            values.second_iteration_values.histogram,
            std::make_shared<kiv_ppr::CNormal_CDF>(values.first_iteration_values.mean, values.second_iteration_values.var)
        );
        return chi_square_normal.Run(2);
    }));

    workers.push_back(std::async(std::launch::async, [&]() {
        kiv_ppr::CChi_Square chi_square_uniform(
            "Uniform",
            std::make_shared<kiv_ppr::CHistogram>(*values.second_iteration_values.histogram),
            std::make_shared<kiv_ppr::CUniform_CDF>(values.first_iteration_values.min, values.first_iteration_values.max)
        );
        return chi_square_uniform.Run(2); // TODO or 0?
    }));

     if (values.first_iteration_values.all_numbers_positive)
     {
         workers.push_back(std::async(std::launch::async, [&]() {
             kiv_ppr::CChi_Square chi_square_exponential(
                 "Exponential",
                 std::make_shared<kiv_ppr::CHistogram>(*values.second_iteration_values.histogram),
                 std::make_shared<kiv_ppr::CExponential_CDF>(1.0 / values.first_iteration_values.mean)
             );
             return chi_square_exponential.Run(1);
         }));
     }

     if (values.first_iteration_values.all_normal_numbers && values.first_iteration_values.mean >= 0)
     {
         workers.push_back(std::async(std::launch::async, [&]() {
             kiv_ppr::CChi_Square chi_square_poisson(
                 "Poisson",
                 std::make_shared<kiv_ppr::CHistogram>(*values.second_iteration_values.histogram),
                 std::make_shared<kiv_ppr::CPoisson_CDF>(values.first_iteration_values.mean)
             );
             return chi_square_poisson.Run(1);
         }));
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

static void Run()
{
    kiv_ppr::CFile_Reader<double> file(filename);
    if (file.Is_Open())
    {
        std::cout << "Processing file " << file.Get_Filename() << " [" << file.Get_File_Size() << " B]\n\n";
        // std::cout << file << "\n";
        kiv_ppr::CFile_Stats file_stats(&file, kiv_ppr::utils::Is_Valid_Double);
        if (0 != file_stats.Process(&kiv_ppr::config::default_thread_params))
        {
            std::cerr << L"Failed to process the input file\n";
            std::exit(1);
        }
        auto values = file_stats.Get_Values();

        std::cout << "Calculated statistics:\n";
        std::cout << values << '\n';

        Run_Statistical_Tests(values);
    }
    else
    {
        std::cerr << L"Failed to open the input file\n";
        std::exit(1);
    }
}

int main()
{
    // kiv_ppr::utils::Generate_Numbers<std::uniform_real_distribution<>>(filename.c_str(), true, 1000000, 5, 10);

    const auto seconds = kiv_ppr::utils::Time_Call([]() {
        Run();
    });
    std::cout << " (" << seconds << " sec)\n";
}