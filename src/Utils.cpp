#include <cmath>

namespace kiv_ppr::utils
{
    bool Double_Valid_Function(double value)
    {
        const auto type = std::fpclassify(value);
        return type == FP_NORMAL || type == FP_ZERO;
    }
}
