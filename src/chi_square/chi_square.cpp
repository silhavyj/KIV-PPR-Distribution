#include <cmath>
#include <utility>

#include "../utils/utils.h"
#include "../config.h"
#include "chi_square.h"

namespace kiv_ppr
{
    CChi_Square::CChi_Square(std::string name,
                             double alpha_critical,
                             std::shared_ptr<CHistogram> histogram,
                             std::shared_ptr<CCDF> cdf)
        : m_name(std::move(name)),
          m_alpha_critical(alpha_critical),
          m_histogram(std::move(histogram)),
          m_cdf(std::move(cdf))
    {

    }

    typename CChi_Square::TResult CChi_Square::Run(int estimated_parameters)
    {
        double E;                             // Expected value
        double O;                             // Actual calculated value
        double chi_square_val = 0.0; // Chi-Square value

        // Get the number of intervals the histogram has originally.
        const size_t original_number_of_intervals = m_histogram->Get_Number_Of_Intervals() - 1;

        double left = m_histogram->Get_Min(); // Left boundary of the interval (L)
        double left_last = 0;                 // Previous left boundary of the interval (the very last interval may need to be merged to the left)
        double right = left;                  // Right boundary of the interval (R)
        size_t number_of_interval = 0;        // Actual number of intervals after the histogram has been shrank.
        double error;                         // Calculated error = ((O-E)^2) / E
        double error_last = 0;                // Previous error (used if the very last interval has to be merged to the left)
        size_t i = 0;                         // Current index (going over all intervals)
        size_t i_last = 0;                    // Previous index 
        size_t i_last_tmp;                    // Temporary index value

        // Goes over all intervals and calculates the Chi-Square value (error).
        while (i < original_number_of_intervals)
        {
            i_last_tmp = i;
            O = 0;

            // If the expected value (E) is < 5, the interval has to be merged
            // with the next interval to its right (unless it is the very last interval).
            do
            {
                // Merge the interval (move its right boundary).
                right += m_histogram->Get_Interval_Size();

                // Calculate the expected value (E).
                E = Calculate_E(right, left, left == m_histogram->Get_Min());

                // Sum up all actual values that fall in between the [left; right].
                O += static_cast<double>(m_histogram->operator[](i));
                ++i;
            } while (i < original_number_of_intervals && E < config::chi_square::Min_Expected_Value);

            // Check if the last interval needs to be merged with the previous one.
            if (E < config::chi_square::Min_Expected_Value)
            {
                // Rollback all values calculated in the previous iteration.
                chi_square_val -= error_last;
                O = 0;
                i = i_last;
                left = left_last;
                right = left;

                // Iterate over the remaining intervals (to the end of the histogram).
                while (i < original_number_of_intervals)
                {
                    right += m_histogram->Get_Interval_Size();
                    O += static_cast<double>(m_histogram->operator[](i));
                    ++i;
                }
                
                // Calculate the final Chi-Square error and exit the loop.
                E = Calculate_E(right, left, left == m_histogram->Get_Min());
                chi_square_val += ((O - E) / E) * (O - E);
                break;
            }

            // Calculate the error and add it to the final Chi-Square value.
            error = ((O - E) / E) * (O - E);
            chi_square_val += error;

            // Store current values in case a rollback will take place.
            error_last = error;
            i_last = i_last_tmp;
            left_last = left;

            // Move on to the next bin (interval).
            left = right;
            ++number_of_interval;
        }

        // Calculate the degrees of freedom as well as the P-value.
        const auto df = static_cast<int>(number_of_interval - 1 - estimated_parameters);
        const double p_value = Calculate_P_Value(chi_square_val, df);

        // Compare the calculated P-value to the critical one and set
        // the result status to either Accepted or Rejected.
        NTResult_Status result_status;
        if (p_value > m_alpha_critical)
        {
            result_status = NTResult_Status::Accepted;
        }
        else
        {
            result_status = NTResult_Status::Rejected;
        }

        // Return the results of the test.
        return { result_status, chi_square_val, p_value, df, m_name };
    }

    double CChi_Square::Calculate_E(double x, double x_prev, bool first_interval) const
    {
        double E;

        // The CDF function results a value between [0; 1]. Therefore, we
        // need to multiply it by the total number of values stored in the histogram.
        const auto count = static_cast<double>(m_histogram->Get_Total_Count());

        // In the first interval, we do not subtract the previous input value.
        if (first_interval)
        {
            E = m_cdf->operator()(x) * count;
        }
        else
        {
            E = (m_cdf->operator()(x) - m_cdf->operator()(x_prev)) * count;
        }

        return E;
    }

    bool CChi_Square::TResult::operator<(const TResult& other) const
    {
        // Compare the results primarily by their p values. If they are
        // the same, compare them by their Chi-Square values (minimal error).
        if (p_value == other.p_value)
        {
            if ((df < 0 && other.df > 0) || (df > 0 && other.df < 0))
            {
                return df > other.df;
            }
            return chi_square < other.chi_square;
        }
        return p_value > other.p_value;
    }

    double CChi_Square::Calculate_P_Value(double x, int df)
    {
        // x = a computed chi-square value.
        // df = degrees of freedom.
        // output = prob. x value occurred by chance.
        // ACM 299.
        if (x <= 0.0 || df < 1)
        {
            return 0;
        }
        double a; // 299 variable names
        double y = 0.0;
        double s;
        double z;
        double ee; // change from e
        double c;
        bool even; // Is df even?
        a = 0.5 * x;
        if (df % 2 == 0)
        {
            even = true;
        }
        else
        {
            even = false;
        }
        if (df > 1)
        {
            y = Exp(-a); // ACM update remark (4)
        }
        if (even)
        {
            s = y;
        }
        else
        {
            s = 2.0 * Gauss(-std::sqrt(x));
        }
        if (df > 2)
        {
            x = 0.5 * (df - 1.0);
            if (even)
            {
                z = 1.0;
            }
            else
            {
                z = 0.5;
            }
            if (a > 40.0) // ACM remark (5)
            {
                if (even)
                {
                    ee = 0.0;
                }
                else
                {
                    ee = 0.5723649429247000870717135;
                }
                c = std::log(a); // log base e
                while (z <= x)
                {
                    ee = std::log(z) + ee;
                    s = s + Exp(c * z - a - ee); // ACM update remark (6)
                    z = z + 1.0;
                }
                return s;
            } // a > 40.0
            else
            {
                if (even)
                {
                    ee = 1.0;
                }
                else
                {
                    ee = 0.5641895835477562869480795 / std::sqrt(a);
                }
                c = 0.0;
                while (z <= x)
                {
                    ee = ee * (a / z); // ACM update remark (7)
                    c = c + ee;
                    z = z + 1.0;
                }
                return c * y + s;
            }
        } // df > 2
        else
        {
            return s;
        }
    }

    double CChi_Square::Gauss(double z)
    {
        // input = z-value (-inf to +inf)
        // output = p under Normal curve from -inf to z
        // ACM Algorithm #209
        double y; // 209 scratch variable
        double p; // result. called z in 209
        double w; // 209 scratch variable
        if (z == 0.0)
        {
            p = 0.0;
        }
        else
        {
            y = std::abs(z) / 2;
            if (y >= 3.0)
            {
                p = 1.0;
            }
            else if (y < 1.0)
            {
                w = y * y;
                p = ((((((((0.000124818987 * w
                    - 0.001075204047) * w + 0.005198775019) * w
                    - 0.019198292004) * w + 0.059054035642) * w
                    - 0.151968751364) * w + 0.319152932694) * w
                    - 0.531923007300) * w + 0.797884560593) * y
                    * 2.0;
            }
            else
            {
                y = y - 2.0;
                p = (((((((((((((-0.000045255659 * y
                    + 0.000152529290) * y - 0.000019538132) * y
                    - 0.000676904986) * y + 0.001390604284) * y
                    - 0.000794620820) * y - 0.002034254874) * y
                    + 0.006549791214) * y - 0.010557625006) * y
                    + 0.011630447319) * y - 0.009279453341) * y
                    + 0.005353579108) * y - 0.002141268741) * y
                    + 0.000535310849) * y + 0.999936657524;
            }
        }
        if (z > 0.0)
        {
            return (p + 1.0) / 2;
        }
        else
        {
            return (1.0 - p) / 2;
        }
    }

    double CChi_Square::Exp(double x)
    {
        if (x < -40.0) // ACM update remark (8)
        {
            return 0.0;
        }
        else
        {
            return std::exp(x);
        }
    }

    std::ostream& operator<<(std::ostream& out, const CChi_Square::TResult& result)
    {
        out << "[test_name=" << result.name << "; chi_square=" << result.chi_square << "; df=" << result.df << "; p_value=" << result.p_value << "]";
        return out;
    }
}

// EOF