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
        enum class ETResult_Status : uint8_t
        {
            ACCEPTED,
            REJECTED
        };

        struct TResult
        {
            ETResult_Status status;
            double chi_square{};
            double p_value{};
            int df{};
            std::string name{};

            bool operator<(const TResult& other) const;
        };

    public:
        CChi_Square(std::string name,
                    double alpha_critical,
                    std::shared_ptr<CHistogram> histogram,
                    std::shared_ptr<CCDF> cdf);

        ~CChi_Square() = default;

        [[nodiscard]] TResult Run(int estimated_parameters);

    public:
        [[nodiscard]] double Calculate_E(double x, double x_prev, bool first_interval) const;

    private:
        std::string m_name;
        double m_alpha_critical;
        std::shared_ptr<CHistogram> m_histogram;
        std::shared_ptr<CCDF> m_cdf;
    };
}