#pragma once

#include "FileReader.h"

namespace kiv_ppr
{
    template<class T, class E>
    class CNormal_Distribution_Test
    {
    public:
        CNormal_Distribution_Test(CFile_Reader<E>* file, std::function<bool(E)> num_valid_fce, T mean, T SD);
        ~CNormal_Distribution_Test() = default;

        [[nodiscard]] int Process(config::TThread_Config thread_config);

    private:
        void Report_Results(size_t category1, size_t category2, size_t category3);
        [[nodiscard]] int Worker(const config::TThread_Config* thread_config) noexcept;

    private:
        CFile_Reader<E>* m_file;
        std::function<bool(E)> m_num_valid_fce;
        T m_mean;
        T m_SD;
        size_t m_category1;
        size_t m_category2;
        size_t m_category3;
        std::mutex m_mtx;
    };
}