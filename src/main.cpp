#include <iostream>
#include <thread>

#include "Utils.h"
#include "Config.h"
#include "FileReader.h"
#include "processing/FileStats1.h"

int main()
{
    std::string filename{"data.dat"};
    // kiv_ppr::utils::Generate_Numbers<std::uniform_real_distribution<>>(filename.c_str(), true, 1024*1024*10, -300, 300);

    kiv_ppr::CFile_Reader<double> file(filename);
    if (file.Is_Open())
    {
        std::cout << "Processing file " << file.Get_Filename() << " [" << file.Get_File_Size() << " B]\n";

        kiv_ppr::CFile_Stats_1 file_stats_1(&file, kiv_ppr::utils::Is_Valid_Double);
        if (0 != file_stats_1.Run(&kiv_ppr::config::default_thread_params))
        {
            std::cerr << L"Failed to process the input file (1)\n";
        }
        std::cout << file_stats_1.Get_Values() << "\n\n";

        std::cout << file_stats_1.Get_Mean() << "\n";
        std::cout << kiv_ppr::utils::Calculate_Mean_Sequential(file, kiv_ppr::config::default_thread_params.number_of_elements_per_file_read) << '\n';
    }
    else
    {
        std::cerr << L"Failed to open the input file\n";
        return 1;
    }
}