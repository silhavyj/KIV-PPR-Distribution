#pragma once

namespace kiv_ppr
{
    /// \author Jakub Silhavy
    ///
    /// This class works as an interface for all CDF functions
    /// within this project. What is a CDF function - https://en.wikipedia.org/wiki/Cumulative_distribution_function.
    class CCDF
    {
    public:
        /// Virtual destructor to ensure proper deallocation when inheriting from this class.
        virtual ~CCDF() = default;
        
        /// Call operator of the class.
        /// This method must be overwritten by all classes that inherit from this class.
        /// \return CDF(x)
        [[nodiscard]] virtual double operator()(double x) const = 0;
    };
}

// EOF