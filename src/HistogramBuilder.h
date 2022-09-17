#pragma once

#include "FileReader.h"
#include "Histogram.h"

namespace kiv_ppr::histogram
{
    template<class T>
    Histogram<T> Build_Histogram(File_Reader<T>* file, uint32_t number_of_slots, T min, T max);
}