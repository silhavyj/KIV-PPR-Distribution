#include <cmath>

#include "Histogram.h"

namespace kiv_ppr::utils
{
    bool Double_Valid_Function(double value)
    {
        const auto type = std::fpclassify(value);
        return type == FP_NORMAL || type == FP_ZERO;
    }

    bool Is_Uniform_Distribution(std::shared_ptr<CHistogram> histogram)
    {
        static constexpr double PERCENTAGE_LIMIT = 0.5; // 10%

        const double lowest_freq = histogram->Get_Lowest_Frequency();
        const double highest_freq = histogram->Get_Highest_Frequency();

        const double diff = highest_freq - lowest_freq;

        /* std::cout << "lo = " << lowest_freq << "\n";
        std::cout << "hi = " << highest_freq << "\n";
        std::cout << "diff = " << diff << "\n";
        std::cout << "limit = " << (highest_freq * PERCENTAGE_LIMIT) << "\n"; */

        return diff <= (highest_freq * PERCENTAGE_LIMIT);
    }

    uint32_t Get_Number_Of_Intervals(size_t n)
    {
        // TODO add an upper limit!!!
        static constexpr size_t MAX_LIMIT = 1024 * 1024 * 5; // 5 MB
        return std::min(static_cast<size_t>(2.82 * std::pow(n, 2.0 / 5.0)), MAX_LIMIT);
    }
}
