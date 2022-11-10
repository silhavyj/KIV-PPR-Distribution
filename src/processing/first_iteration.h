#pragma once

#include <mutex>
#include <utility>
#include <functional>

#include "../config.h"
#include "../utils/file_reader.h"
#include "../utils/watchdog.h"
#include "gpu_kernels.h"

namespace kiv_ppr
{
    /// \author Jakub Silhavy
    ///
    /// This class performs the first iteration over the 
    /// input file. As a result, it calculates the following
    /// statistics: min, max, mean, count (number of valid doubles),
    /// all_ints (whether all numbers are integers or not).
    class CFirst_Iteration
    {
    public:
        /// Used when aggregating means calculated by different worker
        /// threads (we need to know how many values make up the mean).
        using Worker_Mean_t = std::pair<double, size_t>;

        /// Statistical values calculated in the first iteration.
        struct TValues
        {
            double min = std::numeric_limits<double>::max();    ///< Minimum out of all valid doubles in the input file
            double max = std::numeric_limits<double>::lowest(); ///< Maximum out of all valid doubles in the input file
            double mean = 0.0;                                  ///< Mean 
            size_t count = 0;                                   ///< Number of valid double in the input file
            bool all_ints = true;                               ///< Flag indicating whether all values are integers or not
        };

    public:
        /// Creates an instance of the class.
        /// \param file Pointer to an input file reader. 
        explicit CFirst_Iteration(CFile_Reader<double>* file);

        /// Default destructor.
        ~CFirst_Iteration() = default;

        /// Returns statistical values calculated in the first iteration.
        /// \return Statistical values: min, max, mean, count, all_ints
        [[nodiscard]] TValues Get_Values() const noexcept;

        /// Reads the input file and calculates the statistical value.
        /// \param thread_config Configuration containing how many threads should be used to process the input file.
        /// \return 0, if all goes well. 1, if it failed to process the input file.
        [[nodiscard]] int Run(config::TThread_Params* thread_config);

    private:
        /// Report from an OpenCL device after it finishes given work.
        struct TOpenCL_Report
        {
            bool success;       ///< Flag indicating if the device successfully finished given work
            bool all_processed; ///< Flag indicating whether all values have been processed or not
            TValues values;     ///< Calculated values using OpenCL
        };

    private:
        /// Reports local values (from a thread) to the farmer. 
        /// \param values Values calculated by a worker thread.
        void Report_Worker_Results(TValues values);

        /// Worker thread that processes one junk of data from the input file.
        /// After the piece of data is processed, it reports the statistics to the farmer.
        /// \param thread_config Configuration containing the size of a data block processed by each thread
        /// \param watchdog Watchdog the thread periodically reports to (health check)
        /// \return 0, if all went well, 1 otherwise (e.g. failed to read the input file).
        [[nodiscard]] int Worker(config::TThread_Params* thread_config, CWatchdog* watchdog);

        /// Processes a block of data read from the input file on an OpenCL device.
        /// \param opencl OpenCL configuration (device, context, work group size, ...)
        /// \param data_block Block of data to be processed
        /// \return OpenCL report (whether the data was processed successfully or not and how many values were not processed due to the work group size).
        [[nodiscard]] TOpenCL_Report Execute_OpenCL(kernels::TOpenCL_Settings& opencl, CFile_Reader<double>::TData_Block& data_block);

        /// Aggregates values calculated on an OpenCL device.
        /// \param out_min Minimums calculated by individual work groups
        /// \param out_max Maximums calculated by individual work groups
        /// \param out_mean Means calculated by individual work groups
        /// \param out_count Number of valid doubles counted by individual work groups
        /// \param total_count Total number of valid number counted across all work groups
        /// \return Aggregated values calculated on an OpenCL device.
        [[nodiscard]] static TValues Aggregate_Results_From_GPU(const std::vector<double>& out_min,
                                                                const std::vector<double>& out_max, 
                                                                const std::vector<double>& out_mean,
                                                                const std::vector<cl_ulong>& out_count,
                                                                size_t total_count);

        /// Processes a block of data read from the input file on the CPU.
        /// This method directly modifies the local_values structure passed in as a parameter.
        /// \param local_values Local values being calculated within a single worker thread.
        /// \param data_block Block of data to be processed.
        void Execute_On_CPU(TValues& local_values, const CFile_Reader<double>::TData_Block& data_block);

        /// Processes a block of data read from the input file on an OpenCL device.
        /// This method directly modifies the local_values structure passed in as a parameter.
        /// \param local_values Local values being calculated within a single worker thread.
        /// \param data_block Block of data to be processed.
        /// \param opencl OpenCL configuration (device, context, work group size, ...)
        void Execute_On_GPU(TValues& local_values, CFile_Reader<double>::TData_Block& data_block, kernels::TOpenCL_Settings& opencl);

        /// Processes the reaming values that the OpenCL device did not calculate (due to its work group size).
        /// \param data_block Block of data to be processed.
        /// \param offset Number of values the OpenCL device did process (where it should start from).
        /// \return Calculated values (statistics)
        [[nodiscard]] TValues Process_Data_Block_On_CPU(CFile_Reader<double>::TData_Block& data_block, size_t offset);

        /// Merges values calculated on an OpenCL device and on the CPU.
        /// \param dest Destination values that will be modified (result).
        /// \param src The other set of data to be merged into the first set of data.
        void Merge_Values(TValues& dest, const TValues& src);

    private:
        CFile_Reader<double>* m_file;              ///< Pointer to the input file reader
        TValues m_values;                          ///< Statistical values calculated in the first iteration
        std::mutex m_mtx;                          ///< Mutex used in the Farmer-Worker scheme
        std::vector<Worker_Mean_t> m_worker_means; ///< Means calculated by individual workers
    };
}

// EOF