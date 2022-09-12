#include <spdlog/spdlog.h>

#include <thread>
#include <vector>
#include <iostream>
#include <limits>
#include <mutex>

#include "FileReader.h"

auto min_number = std::numeric_limits<double>::max();
auto max_number = std::numeric_limits<double>::min();
std::mutex mtx;

void Report(double min_num, double max_num)
{
    const std::lock_guard<std::mutex> lock(mtx);
    min_number = std::min(min_number, min_num);
    max_number = std::max(max_number, max_num);
}

void Foo(kiv_ppr::File_Reader<double>* file)
{
    while (true)
    {
        const auto block = file->Read_Data();
        auto min_num = std::numeric_limits<double>::max();
        auto max_num = std::numeric_limits<double>::min();

        switch (block.flag)
        {
            case kiv_ppr::File_Reader<double>::Flag::OK:
                for (std::size_t i = 0; i < block.count; ++i)
                {
                   // std::cout << block.data[i] << " ";

                    min_num = std::min(min_num, block.data[i]);
                    max_num = std::max(max_num, block.data[i]);
                }
                Report(min_num, max_num);
                break;

            case kiv_ppr::File_Reader<double>::Flag::ERROR:
                spdlog::error("Could not read data");
                return;

            case kiv_ppr::File_Reader<double>::Flag::_EOF:
                return; // All work is done
        }
    }
}

int main()
{
    //spdlog::set_level(spdlog::level::debug); // Set global log level to debug

    kiv_ppr::File_Reader<double> file("../../data.dat", 100 / sizeof(double));
    if (file.Is_Open())
    {
        std::vector<std::thread> workers;

        for (int i = 0; i < 10; ++i)
        {
            workers.emplace_back(Foo, &file);
        }
        for (auto& worker : workers)
        {
            worker.join();
        }
        std::cout << "\nmin number " << min_number << '\n';
        std::cout << "max number " << max_number << '\n';
    }
    else
    {
        spdlog::error("File not open");
    }

    return 0;
}
