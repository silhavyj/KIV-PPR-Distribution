#include <iostream>

#include "Utils.h"
#include "FileReader.h"
#include "BasicFileStats.h"
#include "Histogram.h"
#include "Config.h"

bool Test_Value(double value)
{
    const auto type = std::fpclassify(value);
    return type == FP_NORMAL || type == FP_ZERO;
}

void Foo(kiv_ppr::File_Reader<double>& file)
{
    file.Seek_Beg();
    bool eof = false;

    while (!eof)
    {
        auto [status, count, data] = file.Read_Data(10);
        switch (status)
        {
            case kiv_ppr::File_Reader<double>::Status::OK:
                for (std::size_t i = 0; i < count; ++i)
                {
                    if (false == Test_Value(data[i]))
                    {
                        std::cout << "Wrong number\n";
                    }
                }
                break;
            case kiv_ppr::File_Reader<double>::Status::ERROR:
                std::cerr << "Error\n";
                return;
            case kiv_ppr::File_Reader<double>::Status::EOF_:
                eof = true;
                break;
        }
    }
}

int main()
{
    std::string filename{"data.dat"};
    kiv_ppr::utils::Generate_Numbers<std::normal_distribution<>>(filename.c_str(), 1000, 0, 2);

    kiv_ppr::File_Reader<double> file(filename);
    if (file.Is_Open())
    {
        // Foo(file);
        // std::cout << file << "\n";

        kiv_ppr::Basic_File_Stats<double, double> basic_stats(&file);

        if (0 == basic_stats.Process())
        {
            const auto [min, max, mean] = basic_stats.Get_Values();

            std::cout << "min = " << min << "\n";
            std::cout << "max = " << max << "\n";
            std::cout << "mean = " << mean << "\n";

            auto histogram = kiv_ppr::Histogram<double>::Generate_Histogram(file, 10, min, max, 2);
            std::cout << histogram << "\n";
        }
        else
        {
            std::cerr << "Error while processing the file\n";
        }
    }
    else
    {
        std::cerr << "File not open\n";
    }
}