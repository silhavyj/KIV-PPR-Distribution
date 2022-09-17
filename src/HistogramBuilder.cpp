#include "HistogramBuilder.h"

namespace kiv_ppr::histogram
{
    template<class T>
    Histogram<T> Build_Histogram(File_Reader<T>* file, uint32_t number_of_slots, T min, T max)
    {
        Histogram<T> histogram(number_of_slots, min, max);

        file->Seek_Beg();
        while (true)
        {
            const auto [status, count, data] = file->Read_Data();
            if (status == kiv_ppr::File_Reader<T>::Status::EOF_)
            {
               // TODO
                break;
            }
            if (status != kiv_ppr::File_Reader<T>::Status::OK)
            {
                break;
            }
            for (std::size_t i = 0; i < count; ++i)
            {
                histogram.Add(data[i]);
            }
        }
        return histogram;
    }

    template Histogram<double> Build_Histogram(File_Reader<double>* file, uint32_t number_of_slots, double min, double max);
}