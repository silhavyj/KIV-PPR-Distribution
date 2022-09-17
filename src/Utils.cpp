#include <fstream>
#include <random>
#include <limits>
#include <ctime>

#include "Utils.h"

namespace kiv_ppr::utils
{
    static std::default_random_engine engine(time(nullptr));

    int Generate_Uniform_Distribution_Test_File(const std::string& filename, std::size_t size, double min, double max)
    {
        const std::size_t number_of_elements = size / sizeof(double);

        std::ofstream file(filename, std::ios::binary);
        if (!file)
        {
            return 1;
        }

        double value;
        std::uniform_real_distribution<double> distribution(min, max);

        for (std::size_t i = 0; i < number_of_elements; ++i)
        {
            value = distribution(engine);
            file.write(reinterpret_cast<char*>(&value), sizeof value);
        }
        return 0;
    }

    int Generate_Normal_Distribution_Test_File(const std::string& filename, std::size_t size, double mean, double SD)
    {
        const std::size_t number_of_elements = size / sizeof(double);
        std::ofstream file(filename, std::ios::binary);
        if (!file)
        {
            return 1;
        }
        double value;
        std::normal_distribution<double> distribution(mean, SD);

        for (std::size_t i = 0; i < number_of_elements; ++i)
        {
            value = distribution(engine);
            file.write(reinterpret_cast<char*>(&value), sizeof value);
        }
        return 0;
    }
}
