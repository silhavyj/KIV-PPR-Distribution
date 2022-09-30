#pragma once

#include <string>
#include <memory>
#include <iostream>

#include "../cdfs/Cdf.h"
#include "../Histogram.h"

namespace kiv_ppr
{
    class CChi_Square
    {
    public:
        struct TResult
        {
            double critical_value;
            std::string name;

            bool operator<(const TResult& other) const;
            friend std::ostream& operator<<(std::ostream& out, const TResult& result);
        };

    public:
        CChi_Square(const std::string& name,
                    const std::shared_ptr<CHistogram>& histogram,
                    const std::shared_ptr<CCDF>& cdf);

        ~CChi_Square() = default;

        [[nodiscard]] TResult Run();

    private:
        std::string m_name;
        std::shared_ptr<CHistogram> m_histogram;
        std::shared_ptr<CCDF> m_cdf;
    };
}