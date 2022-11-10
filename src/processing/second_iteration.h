#pragma once

#include <memory>
#include <functional>

#include "first_iteration.h"
#include "histogram.h"
#include "../utils/file_reader.h"
#include "../utils/watchdog.h"

namespace kiv_ppr
{
    /// \author Jakub Silhavy
    ///
    /// This class performs the second iteration over the 
    /// input file. As a result, it calculates the following
    /// statistics: variance, standard deviation, and histogram, which
    /// is used for the Chi-Square goodness of fit test.
    class CSecond_Iteration
    {
    public:
        /// Statistical values calculated within the second iteration.
        struct TValues
        {
            double var = 0.0;                                ///< Variance
            double sd = 0.0;                                 ///< Standard deviation
            std::shared_ptr<CHistogram> histogram = nullptr; ///< Histogram
        };

    public:
        /// Creates and instance of the class.
        /// \param file Pointer to an input file reader. 
        /// \param basic_values Values calculated in the first iteration (min, max, mean, ...)
        explicit CSecond_Iteration(CFile_Reader<double>* file,
                                   typename CFirst_Iteration::TValues* basic_values);

        /// Default destructor.
        ~CSecond_Iteration() = default;

        /// Returns statistical values calculated in the second iteration.
        /// \return Statistical values: variance, standard deviation, histogram
        [[nodiscard]] TValues Get_Values() const noexcept;

        /// Reads the input file and calculates the statistical values.
        /// \param thread_config Configuration containing how many threads should be used to process the input file.
        /// \return 0, if all goes well. 1, if it failed to process the input file. 
        [[nodiscard]] int Run(config::TThread_Params* thread_config);

    private:
        /// Report from an OpenCL device after it finishes given work.
        struct TOpenCL_Report
        {
            bool success;       ///< Flag indicating if the device successfully finished given work
            bool all_processed; ///< Flag indicating whether all values have been processed or not
        };

    private:
        /// Reports local values (from a thread) to the farmer. 
        /// \param values Values calculated by a worker thread.
        void Report_Worker_Results(const TValues& values);

        /// Worker thread that processes one junk of data from the input file.
        /// After the piece of data is processed, it reports the statistics to the farmer.
        /// \param thread_config Configuration containing the size of a data block processed by each thread
        /// \param watchdog Watchdog the thread periodically reports to (health check)
        /// \return 0, if all went well, 1 otherwise (e.g. failed to read the input file).
        [[nodiscard]] int Worker(const config::TThread_Params* thread_config, CWatchdog* watchdog);

        /// Helper function that calculates how many intervals should make up the histogram
        /// based on the total number of valid doubles.
        /// Idea taken from: 
        /// https://onlinelibrary.wiley.com/doi/full/10.1002/1097-0320%2820011001%2945%3A2%3C141%3A%3AAID-CYTO1156%3E3.0.CO%3B2-M#bib11
        /// \param n Number of values
        /// \return Number of intervals
        [[nodiscard]] static size_t Calculate_Number_Of_Intervals(size_t n) noexcept;

        /// Scales up the basic values calculated in the first iteration.
        /// If the minimum >= 0, we multiple the values as they were before scaling down in the first iteration.
        /// It is done due to the poisson distribution, which cannot be scaled down without affecting the result
        /// of the Chi-Square goodness of fit test (the poisson distribution is not "scalable").
        /// \param basic_values Statistical values calculated in the first iteration
        void Scale_Up_Basic_Values(typename CFirst_Iteration::TValues* basic_values) noexcept;

        /// Processes a block of data read from the input file on the CPU.
        /// This method directly modifies the local_values structure passed in as a parameter.
        /// \param local_values Local values being calculated within a single worker thread.
        /// \param data_block Block of data to be processed.
        /// \param offset Offset (remaining part of data that could not be calculated on the OpenCL device).
        void Execute_On_CPU(TValues& local_values, const CFile_Reader<double>::TData_Block& data_block, size_t offset = 0) noexcept;

        /// Processes a block of data read from the input file on an OpenCL device.
        /// This method directly modifies the local_values structure passed in as a parameter.
        /// \param local_values Local values being calculated within a single worker thread.
        /// \param data_block Block of data to be processed.
        /// \param opencl OpenCL configuration (device, context, work group size, ...)
        void Execute_On_GPU(TValues& local_values, const CFile_Reader<double>::TData_Block& data_block, kernels::TOpenCL_Settings& opencl);

        /// Processes a block of data read from the input file on an OpenCL device.
        /// \param opencl OpenCL configuration (device, context, work group size, ...)
        /// \param data_block Block of data to be processed
        /// \param local_values Statistical values being calculated in the second interation (var, sd, histogram)
        /// \return  OpenCL report (whether the data was processed successfully or not and how many values were not processed due to the work group size).
        [[nodiscard]] TOpenCL_Report Execute_OpenCL(kernels::TOpenCL_Settings& opencl, const CFile_Reader<double>::TData_Block& data_block, TValues& local_values);

    private:
        CFile_Reader<double>* m_file;                       ///< Pointer to the input file reader
        typename CFirst_Iteration::TValues* m_basic_values; ///< Statistical values calculated in the first iteration
        TValues m_values;                                   ///< Statistical values calculated in the second iteration
        std::mutex m_mtx;                                   ///< Mutex used in the Farmer-Worker scheme
        CHistogram::TParams m_histogram_params;             ///< Histogram parameters
    };
}

// EOF