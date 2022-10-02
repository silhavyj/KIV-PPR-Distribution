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
            double chi_square;
            double p_value;
            size_t categories;
            std::string name;

            bool operator<(const TResult& other) const;
            friend std::ostream& operator<<(std::ostream& out, const TResult& result);
        };

    public:
        CChi_Square(std::string name,
                    std::shared_ptr<CHistogram> histogram,
                    std::shared_ptr<CCDF> cdf);

        ~CChi_Square() = default;

        [[nodiscard]] TResult Run(int estimated_parameters);

    public:
        [[nodiscard]] double Calculate_E(double x, double x_prev, bool first_interval) const;
        [[nodiscard]] static double Calculate_P_Value(double x, int df);
        [[nodiscard]] static double Exp(double x);
        [[nodiscard]] static double Gauss(double z);

    private:
        std::string m_name;
        std::shared_ptr<CHistogram> m_histogram;
        std::shared_ptr<CCDF> m_cdf;
    };
}