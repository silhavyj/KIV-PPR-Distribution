#pragma once

namespace kiv_ppr
{
    class CCDF
    {
    public:
        virtual ~CCDF() = default;
        
        [[nodiscard]] virtual double operator()(double x) const = 0;
    };
}