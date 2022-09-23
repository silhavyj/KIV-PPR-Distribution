#pragma once

#include "FileReader.h"

namespace kiv_ppr
{
    class CNormal_Distribution_Test
    {
    public:
        CNormal_Distribution_Test(CFile_Reader<double>* file, std::function<bool(double)> num_valid_fce, double mean, double SD);
        ~CNormal_Distribution_Test() = default;

        [[nodiscard]] int Process(config::TThread_Config thread_config);
        bool Is_Normal_Distribution(double tolerance);
        [[nodiscard]] double Get_Category_68() const noexcept;
        [[nodiscard]] double Get_Category_95() const noexcept;
        [[nodiscard]] double Get_Category_99_7() const noexcept;

    private:
        void Report_Results(size_t category_68, size_t category_95, size_t category_99_7);
        [[nodiscard]] int Worker(const config::TThread_Config* thread_config) noexcept;
        [[nodiscard]] static bool Is_Within_Tolerance(double expected, double actual, double percentage) noexcept;

    private:
        CFile_Reader<double>* m_file;
        std::function<bool(double)> m_num_valid_fce;
        double m_mean;
        double m_SD;
        size_t m_category_68;
        size_t m_category_95;
        size_t m_category_99_7;
        std::mutex m_mtx;
    };
}