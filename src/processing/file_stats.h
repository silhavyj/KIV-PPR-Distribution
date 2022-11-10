#pragma once

#include <memory>
#include <iostream>

#include "first_iteration.h"
#include "second_iteration.h"
#include "../utils/file_reader.h"
#include "histogram.h"
#include "../config.h"

namespace kiv_ppr
{
    /// \author Jakub Silhavy
    ///
    /// This class calculates different statistics from the input file.
    /// These statistics are: min, max, mean, variance, standard deviation,
    /// number of valid doubles, whether or not all values are integers,
    /// and a histogram, which is then used to test out different hypothesis
    /// as to what distribution the input data comes from.
    class CFile_Stats
    {
    public:
        /// Statistical values calculated from the input file.
        struct TValues
        {
            CFirst_Iteration::TValues first_iteration;   ///< Values calculated in the first iteration (min, max, mean, all_ints, count).
            CSecond_Iteration::TValues second_iteration; ///< Values calculated in the second iteration (variance, sd, histogram).

            /// Prints out values in a string format into an output stream.
            /// \param out Output stream.
            /// \param values Statistical values to be printed out into the output stream.
            /// \return Output stream.
            friend std::ostream& operator<<(std::ostream& out, const TValues& values);
        };

    public:
        /// Creates an instance of the class.
        /// \param file Pointer to an input file reader. 
        explicit CFile_Stats(CFile_Reader<double>* file) noexcept;

        /// Default destructor.
        ~CFile_Stats() = default;

        /// Returns statistical values calculated from the input file.
        /// \return Statistical values: min, max, mean, count, all_ints, variance, sd, histogram.
        [[nodiscard]] TValues Get_Values() const noexcept;

        /// Calculates statistical values from the input file. It is carried out
        /// in two iterations (the file is read up twice).
        /// \param thread_config Configuration containing how many threads should be used to process the input file.
        /// \return 0, if all goes well. 1, if it failed to process the input file.
        [[nodiscard]] int Process(config::TThread_Params* thread_config);

    private:
        CFile_Reader<double>* m_file; ///< Pointer to an input file reader.
        TValues m_values;             ///< Statistical values calculated from the input file.
    };
}