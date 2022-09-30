#include "ChiSquare.h"

#include <utility>

namespace kiv_ppr
{
    CChi_Square::CChi_Square(std::string  name, std::shared_ptr<CHistogram>  histogram, std::shared_ptr<CCDF>  cdf)
            : m_name(std::move(name)),
              m_histogram(std::move(histogram)),
              m_cdf(std::move(cdf))
    {

    }

    typename CChi_Square::TResult CChi_Square::Run()
    {
        double critical_value = 0.0;
        const auto total_count = static_cast<double>(m_histogram->Get_Total_Count());
        const size_t number_of_intervals = m_histogram->Get_Number_Of_Intervals();

        // TODO make sure there's at least N groups

        double middle_value;
        double middle_value_prev;
        double E;
        double O;

        for (size_t i = 0; i < number_of_intervals; ++i)
        {
            middle_value = m_histogram->Get_Min() + (static_cast<double>(i) * m_histogram->Get_Interval_Size()) + (m_histogram->Get_Interval_Size() / 2.0);
            if (i != 0)
            {
                E = (m_cdf->operator()(middle_value) - m_cdf->operator()(middle_value_prev));
            }
            else
            {
                E = m_cdf->operator()(middle_value);
            }
            E *= total_count;
            O = static_cast<double>(m_histogram->operator[](i));

            if (E != 0)
            {
                critical_value += ((O - E) * (O - E)) / E;
            }
            middle_value_prev = middle_value;
        }
        return { critical_value, m_name };
    }

    bool CChi_Square::TResult::operator<(const TResult& other) const
    {
        return critical_value < other.critical_value;
    }

    std::ostream& operator<<(std::ostream& out, const CChi_Square::TResult& result)
    {
        out << "[test_name=" << result.name << "; cv=" << result.critical_value << "]";
        return out;
    }
}