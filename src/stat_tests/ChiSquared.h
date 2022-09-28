#pragma once

#include <memory>
#include <string>

#include "../cdfs/Cdf.h"
#include "../Histogram.h"

namespace kiv_ppr
{
    class CChi_Squared
    {
    public:
        CChi_Squared(const std::string& name, const std::shared_ptr<CHistogram>& histogram, const std::shared_ptr<CCDF>& cdf);
        ~CChi_Squared() = default;

        void Run();

    private:
        std::string m_name;
        std::shared_ptr<CHistogram> m_histogram;
        std::shared_ptr<CCDF> m_cdf;
    };
}