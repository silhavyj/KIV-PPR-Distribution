#include <cmath>
#include <utility>

#include "../utils/Utils.h"
#include "ChiSquare.h"

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
        static constexpr double MIN_EXPECTED_VALUE = 5;

        double E;
        double O;
        double chi_square_val = 0.0;
        const size_t original_number_of_intervals = m_histogram->Get_Number_Of_Intervals();

        double left = m_histogram->Get_Min();
        double left_last = 0;
        double right = left;
        size_t number_of_interval = 0;
        double error;
        double error_last = 0;
        size_t i = 0;
        size_t i_last = 0;
        size_t i_last_tmp;

        while (i < original_number_of_intervals)
        {
            i_last_tmp = i;
            O = 0;
            do
            {
                right += m_histogram->Get_Interval_Size();
                E = Calculate_E(right, left, left == m_histogram->Get_Min());
                O += static_cast<double>(m_histogram->operator[](i));
                ++i;
            } while (i < original_number_of_intervals && E < MIN_EXPECTED_VALUE);

            if (E < MIN_EXPECTED_VALUE)
            {
                chi_square_val -= error_last;
                O = 0;
                i = i_last;
                left = left_last;
                right = left;

                while (i < original_number_of_intervals)
                {
                    right += m_histogram->Get_Interval_Size();
                    O += static_cast<double>(m_histogram->operator[](i));
                    ++i;
                }
                E = Calculate_E(right, left, left == m_histogram->Get_Min());
                chi_square_val += ((O - E) / E) * (O - E);
                break;
            }

            error = ((O - E) / E) * (O - E);
            chi_square_val += error;

            error_last = error;
            i_last = i_last_tmp;
            left_last = left;

            left = right;
            ++number_of_interval;
        }

        const auto df = static_cast<int>(number_of_interval - 1 - estimated_parameters);
        const double p_value = utils::Calculate_P_Value(chi_square_val, df);

        ETResult_Status result_status;
        if (p_value > m_alpha_critical)
        {
            result_status = ETResult_Status::ACCEPTED;
        }
        else
        {
            result_status = ETResult_Status::REJECTED;
        }
        return { result_status, chi_square_val, p_value, df, m_name };
    }

    double CChi_Square::Calculate_E(double x, double x_prev, bool first_interval) const
    {
        double E;
        const auto count = static_cast<double>(m_histogram->Get_Total_Count());

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
        return p_value > other.p_value;
    }

    std::ostream& operator<<(std::ostream& out, const CChi_Square::TResult& result)
    {
        out << "[test_name=" << result.name << "; chi_square=" << result.chi_square << "; df=" << result.df << "; p_value=" << result.p_value << "]";
        return out;
    }
}