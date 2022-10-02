#include <cmath>
#include <utility>

#include "ChiSquare.h"

namespace kiv_ppr
{
    CChi_Square::CChi_Square(std::string name,
                             std::shared_ptr<CHistogram> histogram,
                             std::shared_ptr<CCDF> cdf)
            : m_name(std::move(name)),
              m_histogram(std::move(histogram)),
              m_cdf(std::move(cdf))
    {

    }

    typename CChi_Square::TResult CChi_Square::Run(int estimated_parameters)
    {
        static constexpr double MIN_EXPECTED_VALUE = 5;

        double E;
        double O;
        double chi_square_val = 0.0;
        const auto total_count = static_cast<double>(m_histogram->Get_Total_Count());
        const size_t original_number_of_intervals = m_histogram->Get_Number_Of_Intervals();

        size_t i = 0;
        double left = m_histogram->Get_Min();
        double right = left;
        size_t number_of_interval = 0;

        while (i < original_number_of_intervals)
        {
            O = 0;
            do
            {
                right += m_histogram->Get_Interval_Size();
                E = (m_cdf->operator()(right) - m_cdf->operator()(left)) * total_count;
                O += static_cast<double>(m_histogram->operator[](i));
                ++i;
            } while (i < original_number_of_intervals && E < MIN_EXPECTED_VALUE);

            if (E < MIN_EXPECTED_VALUE)
            {
                // TODO merge the last interval with the previous one
                break;
            }

            chi_square_val += ((O - E) / E) * (O - E);

            left = right;
            ++number_of_interval;
        }

        const double p_value = Calculate_P_Value(chi_square_val, static_cast<int>(number_of_interval - 1 - estimated_parameters));
        return { chi_square_val, p_value, number_of_interval, m_name };
    }

    double CChi_Square::Calculate_P_Value(double x, int df)
    {
        // x = a computed chi-square value.
        // df = degrees of freedom.
        // output = prob. x value occurred by chance.
        // ACM 299.
        if (x <= 0.0 || df < 1)
        {
            // TODO throw an exception
        }
        double a; // 299 variable names
        double y = 0.0;
        double s;
        double z;
        double ee; // change from e
        double c;
        bool even; // Is df even?
        a = 0.5 * x;
        if (df % 2 == 0) even = true; else even = false;
        if (df > 1) y = Exp(-a); // ACM update remark (4)
        if (even) s = y;
        else s = 2.0 * Gauss(-std::sqrt(x));
        if (df > 2)
        {
            x = 0.5 * (df - 1.0);
            if (even) z = 1.0; else z = 0.5;
            if (a > 40.0) // ACM remark (5)
            {
                if (even) ee = 0.0;
                else ee = 0.5723649429247000870717135;
                c = std::log(a); // log base e
                while (z <= x) {
                    ee = std::log(z) + ee;
                    s = s + Exp(c * z - a - ee); // ACM update remark (6)
                    z = z + 1.0;
                }
                return s;
            } // a > 40.0
            else
            {
                if (even) ee = 1.0;
                else
                    ee = 0.5641895835477562869480795 / std::sqrt(a);
                c = 0.0;
                while (z <= x) {
                    ee = ee * (a / z); // ACM update remark (7)
                    c = c + ee;
                    z = z + 1.0;
                }
                return c * y + s;
            }
        } // df > 2
        else {
            return s;
        }
    }

    double CChi_Square::Gauss(double z)
    {
        // input = z-value (-inf to +inf)
        // output = p under Normal curve from -inf to z
        // ACM Algorithm #209
        double y; // 209 scratch variable
        double p; // result. called ‘z’ in 209
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

    inline double CChi_Square::Exp(double x)
    {
        if (x < -40.0) // ACM update remark (8)
            return 0.0;
        else
            return std::exp(x);
    }

    bool CChi_Square::TResult::operator<(const TResult& other) const
    {
        return p_value > other.p_value;
    }

    std::ostream& operator<<(std::ostream& out, const CChi_Square::TResult& result)
    {
        out << "[test_name=" << result.name << "; chi_square=" << result.critical_value  << "; categories=" << result.categories << "; p_value=" << result.p_value << "]";
        return out;
    }
}