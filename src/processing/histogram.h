#pragma once

#include <vector>
#include <cstdint>
#include <cstddef>
#include <iostream>
#include <functional>

namespace kiv_ppr
{
    /// \author Jakub Silhavy
    ///
    /// This class represents a histogram that is used 
    /// for the Chi-Square goodness of fit test. It it created
    /// within the second iteration (reading of the input file).
    class CHistogram
    {
    public:
        /// Parameters of the histogram
        struct TParams
        {
            double min;                 ///< Minimum number (left boundary)
            double max;                 ///< Maximum number (right boundary)
            size_t number_of_intervals; ///< Number of intervals (bins)
        };

    public:
        /// Creates an instance of the class. 
        /// \param params Histogram parameters
        explicit CHistogram(TParams params);

        /// Default destructor.
        ~CHistogram() = default;

        /// Adds a number into the histogram.
        /// \param value Value to be added into the histogram.
        void Add(double value) noexcept;

        /// Increments the value at a particular index by the values passed 
        /// as a parameter (histogram[index] += value).
        /// \param index Index (position in the histogram)
        /// \param value Value to be added to the value at the index
        /// \return true, if all went well, false otherwise.
        [[nodiscard]] bool Add(size_t index, size_t value) noexcept;

        /// Returns the number of intervals that make up the histogram.
        /// \return Number of intervals of the histogram.
        [[nodiscard]] size_t Get_Number_Of_Intervals() const noexcept;

        /// Returns the minimum of the histogram (left boundary).
        /// Note: It is not the bin with the fewest elements.
        /// \return Minimum of the histogram
        [[nodiscard]] double Get_Min() const noexcept;

        /// Returns the interval size (width of a bin). 
        /// \return Interval size 
        [[nodiscard]] double Get_Interval_Size() const noexcept;

        /// Returns the total number of values inserted into the histogram.
        /// \return Number of values stored in the histogram
        [[nodiscard]] size_t Get_Total_Count() const noexcept;

        /// Overloaded [] operator for accessing individual intervals (bins).
        /// \param index Index (position in the histogram)
        /// \return Reference to the value at the index position
        [[nodiscard]] size_t& operator[](size_t index) noexcept;

        /// Returns the value at the position given as a parameter.
        /// \param index Index (position in the histogram)
        /// \return Value at the index position
        [[nodiscard]] size_t at(size_t index) const noexcept;

        /// Overloaded += operator fro merging two histograms.
        /// \param other Other histogram to be merged into this one.
        void operator+=(CHistogram& other) noexcept;

        /// Prints out the histogram into an output stream.
        /// \param out Output stream
        /// \param histogram Histogram to be printed out
        /// \return Output stream
        friend std::ostream& operator<<(std::ostream& out, CHistogram& histogram);

    private:
        std::vector<size_t> m_intervals; ///< Intervals (bins) that make up the histogram
        double m_interval_size;          ///< Width of a bin (interval size)
        TParams m_params;                ///< Parameters of the histogram (min, max, number of intervals)
        size_t m_count;                  ///< Total number of values inserted into the histogram
    };
}

// EOF