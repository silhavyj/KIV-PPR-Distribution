#include <iostream>
#include <thread>

#include "utils/Utils.h"
#include "Config.h"
#include "FileReader.h"
#include "processing/FileStats.h"

static std::string filename{"data.dat"};

static void Run()
{
    kiv_ppr::CFile_Reader<double> file(filename);
    if (file.Is_Open())
    {
        std::cout << "Processing file " << file.Get_Filename() << " [" << file.Get_File_Size() << " B]\n";
        kiv_ppr::CFile_Stats file_stats(&file, kiv_ppr::utils::Is_Valid_Double);
        if (0 != file_stats.Run(&kiv_ppr::config::default_thread_params))
        {
            std::cerr << L"Failed to process the input file\n";
            std::exit(1);
        }
        auto values = file_stats.Get_Values();
        std::cout << values << '\n';
    }
    else
    {
        std::cerr << L"Failed to open the input file\n";
        std::exit(1);
    }
}

int main()
{
    // kiv_ppr::utils::Generate_Numbers<std::normal_distribution<>>(filename.c_str(), true, 100000, 200, 20);

    std::cout << kiv_ppr::utils::Time_Call([]() {
        Run();
    }) << " sec\n";
}