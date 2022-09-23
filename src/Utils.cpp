#include <cmath>

#include "Histogram.h"

namespace kiv_ppr::utils
{
    bool Double_Valid_Function(double value)
    {
        const auto type = std::fpclassify(value);
        return type == FP_NORMAL || type == FP_ZERO;
    }

    bool Is_Uniform_Distribution(std::shared_ptr<CHistogram<double>> histogram)
    {
        static constexpr double PERCENTAGE_LIMIT = 0.5; // 10%

        const double lowest_freq = histogram->Get_Lowest_Frequency();
        const double highest_freq = histogram->Get_Highest_Frequency();

        const double diff = highest_freq - lowest_freq;

        std::cout << "lo = " << lowest_freq << "\n";
        std::cout << "hi = " << highest_freq << "\n";
        std::cout << "diff = " << diff << "\n";
        std::cout << "limit = " << (highest_freq * PERCENTAGE_LIMIT) << "\n";

        return diff <= (highest_freq * PERCENTAGE_LIMIT);
    }
}
