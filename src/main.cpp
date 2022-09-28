#include <iostream>
#include <thread>

#include "utils/Utils.h"
#include "Config.h"
#include "FileReader.h"
#include "processing/FileStats1.h"
#include "processing/FileStats2.h"

int main()
{
    std::string filename{"data.dat"};
    kiv_ppr::utils::Generate_Numbers<std::normal_distribution<>>(filename.c_str(), true, 10000, 200, 20);

    kiv_ppr::CFile_Reader<double> file(filename);
    if (file.Is_Open())
    {
        std::cout << "Processing file " << file.Get_Filename() << " [" << file.Get_File_Size() << " B]\n";
        // std::cout << file << '\n';

        kiv_ppr::CFile_Stats_1 file_stats_1(&file, kiv_ppr::utils::Is_Valid_Double);
        if (0 != file_stats_1.Run(&kiv_ppr::config::default_thread_params))
        {
            std::cerr << L"Failed to process the input file (1)\n";
        }
        std::cout << file_stats_1.Get_Values() << "\n";

        // Test if mean is being calculated correctly
        // std::cout << "\n";
        // std::cout << file_stats_1.Get_Mean() << "\n";
        // std::cout << kiv_ppr::utils::Calculate_Mean_Sequential(file, kiv_ppr::config::default_thread_params.number_of_elements_per_file_read) << '\n';

        kiv_ppr::CFile_Stats_2 file_stats_2(&file, kiv_ppr::utils::Is_Valid_Double, file_stats_1.Get_Values());
        if (0 != file_stats_2.Run(&kiv_ppr::config::default_thread_params))
        {
            std::cerr << L"Failed to process the input file (2)\n";
        }
        std::cout << file_stats_2.Get_Values() << "\n";

        auto histogram = file_stats_2.Get_Histogram();
        histogram->Merge_Sparse_Intervals(kiv_ppr::config::BUCKET_MIN_LIMIT);
        std::cout << "histogram = " << *histogram << '\n';
    }
    else
    {
        std::cerr << L"Failed to open the input file\n";
        return 1;
    }
}