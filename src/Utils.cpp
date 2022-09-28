#include "Utils.h"

namespace kiv_ppr::utils
{
    bool Is_Valid_Double(double value)
    {
        const auto type = std::fpclassify(value);
        return type == FP_NORMAL || type == FP_ZERO;
    }

    double Calculate_Mean_Sequential(kiv_ppr::CFile_Reader<double>& file, uint32_t number_of_elements_per_file_read)
    {
        double sum = 0;
        size_t total_count = 0;

        file.Seek_Beg();
        while (true)
        {
            const auto [status, count, data] = file.Read_Data(number_of_elements_per_file_read);
            if (status != kiv_ppr::CFile_Reader<double>::NRead_Status::OK)
            {
                break;
            }
            for (size_t i = 0; i < count; ++i)
            {
                sum += data[i];
            }
            total_count += count;
        }
        return sum / total_count;
    }
}