#include "utils.h"

namespace kiv_ppr::utils
{
    bool Is_Valid_Double(double value) noexcept
    {
        const auto type = std::fpclassify(value);
        return type == FP_NORMAL || type == FP_ZERO;
    }

    namespace vectorization
    {
        double Aggregate(const __m256d& vals, double default_value, std::function<double(double, double)> fce)
        {
            double result = default_value;

#pragma warning(disable:26490)
            // So we have access, to individual values.
            const auto raw_values = reinterpret_cast<const double*>(&vals);
#pragma warning(default:26490)

            // Aggregate the values.
            for (std::size_t i = 0; i < 4; ++i)
            {
                result = fce(result, raw_values[i]);
            }

            return result;
        }

        __m256d Create_4Doubles(std::array<double, 4>& data, const std::size_t offset, double value)
        {
            for (std::size_t i = offset; i < 4; ++i)
            {
                data.at(i) = value;
            }

            return _mm256_set_pd(
                data.at(0),
                data.at(1),
                data.at(2),
                data.at(3)
            );
        }
    }
}

// EOF