#include "utils.h"

namespace kiv_ppr::utils
{
    bool Is_Valid_Double(double value)
    {
        const auto type = std::fpclassify(value);
        return type == FP_NORMAL || type == FP_ZERO;
    }
}