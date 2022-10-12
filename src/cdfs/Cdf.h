#pragma once

namespace kiv_ppr
{
    class CCDF
    {
    public:
        virtual ~CCDF() = default;
        
        [[nodiscard]] virtual double operator()(long double x) const = 0;
    };
}