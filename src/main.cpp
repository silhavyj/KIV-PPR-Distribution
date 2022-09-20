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

    /* std::cout << "Generating file...\n";
    kiv_ppr::utils::Generate_Numbers<std::normal_distribution<>>(filename.c_str(), 268435456, 100, 20);
    std::cout << "Done"; */

    kiv_ppr::File_Reader<double> file(filename);
    if (file.Is_Open())
    {
        // Foo(file);
        // std::cout << file << "\n";

        kiv_ppr::Basic_File_Stats<double, double> basic_stats(&file);

        if (0 == basic_stats.Process(kiv_ppr::config::NUMBER_OF_THREADS_TO_READ_FILE))
        {
            const auto [min, max, mean] = basic_stats.Get_Values();

            std::cout << "min = " << min << "\n";
            std::cout << "max = " << max << "\n";
            std::cout << "mean = " << mean << "\n";

            auto histogram = kiv_ppr::Histogram<double>::Generate_Histogram(file, {kiv_ppr::config::DEFAULT_NUMBER_OF_SLOTS, min, max}, kiv_ppr::config::NUMBER_OF_THREADS_TO_READ_FILE);
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