#include <iostream>
#include <cmath>

#include "ChiSquared.h"

namespace kiv_ppr
{
    double approx_gamma(double Z)
    {
        const double RECIP_E = 0.36787944117144232159552377016147;  // RECIP_E = (E^-1) = (1.0 / E)
        const double TWOPI = 6.283185307179586476925286766559;  // TWOPI = 2.0 * PI

        double D = 1.0 / (10.0 * Z);
        D = 1.0 / ((12 * Z) - D);
        D = (D + Z) * RECIP_E;
        D = pow(D, Z);
        D *= sqrt(TWOPI / Z);

        return D;
    }


    static double igf(double S, double Z)
    {
        if(Z < 0.0)
        {
            return 0.0;
        }
        double Sc = (1.0 / S);
        Sc *= pow(Z, S);
        Sc *= exp(-Z);

        double Sum = 1.0;
        double Nom = 1.0;
        double Denom = 1.0;

        for(int I = 0; I < 200; I++)
        {
            Nom *= Z;
            S++;
            Denom *= S;
            Sum += (Nom / Denom);
        }

        return Sum * Sc;
    }

    static double chisqr(int Dof, double Cv)
    {
        if(Cv < 0 || Dof < 1)
        {
            return 0.0;
        }
        double K = ((double)Dof) * 0.5;
        double X = Cv * 0.5;
        if(Dof == 2)
        {
            return std::exp(-1.0 * X);
        }

        double PValue = igf(K, X);
        if(std::isnan(PValue) || std::isinf(PValue) || PValue <= 1e-8)
        {
            return 1e-14;
        }

        PValue /= approx_gamma(K);
        //PValue /= tgamma(K);

        return (1.0 - PValue);
    }

    CChi_Squared::CChi_Squared(const std::string& name, const std::shared_ptr<CHistogram>& histogram, const std::shared_ptr<CCDF>& cdf)
        : m_name(name),
          m_histogram(histogram),
          m_cdf(cdf)
    {

    }

    void CChi_Squared::Run()
    {
        double critical_value = 0.0;
        const size_t count = m_histogram->Get_Sum_Of_All_Frequencies();
        const size_t number_of_groups = m_histogram->Get_Number_Of_Intervals();
        // TODO make sure there's at least N groups

        double middle_value;
        double middle_value_prev = 0;
        double E;
        double O;

        for (size_t i = 0; i < number_of_groups; ++i)
        {
            middle_value = m_histogram->Get_Min() + (i * m_histogram->Get_Interval_Size()) + (m_histogram->Get_Interval_Size() / 2.0);
            // std::cout << m_histogram->Get_Min() << "\n";
            if (i != 0)
            {
                E = (m_cdf->operator()(middle_value) - m_cdf->operator()(middle_value_prev));
            }
            else
            {
                E = m_cdf->operator()(middle_value);
            }
            E *= count;

            O = m_histogram->operator[](i);

            critical_value += ((O - E) * (O - E)) / E;
            middle_value_prev = middle_value;

            // std::cout << "[middle_value=" << middle_value << "; O=" << O << "; E=" << E << "; critical_value=" << critical_value << "]\n";
        }

        double p_value = chisqr(number_of_groups, critical_value);
        std::cout << p_value << "\n";
        std::cout << "[name=" << m_name << "; critical_value= " << critical_value << "]\n";
    }
}