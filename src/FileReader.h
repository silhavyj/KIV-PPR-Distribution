#pragma once

#include <cstdint>
#include <mutex>
#include <fstream>
#include <string>
#include <memory>
#include <functional>

#include "Config.h"

namespace kiv_ppr
{
    template<class T>
    class CFile_Reader
    {
    public:
        enum class NStatus : uint8_t
        {
            OK,
            ERROR,
            EOF_
        };

        struct TData_Block
        {
            NStatus status;
            long count;
            std::shared_ptr<T[]> data;
        };

    public:
        explicit CFile_Reader(const std::string& filename);
        ~CFile_Reader();

        [[nodiscard]] TData_Block Read_Data(size_t number_of_elements);
        [[nodiscard]] bool Is_Open() const;
        [[nodiscard]] size_t Get_Total_Number_Of_Elements() const noexcept;
        [[nodiscard]] size_t Get_Total_Number_Of_Valid_Elements() const noexcept;
        [[nodiscard]] size_t Get_File_Size() const noexcept;
        [[nodiscard]] std::string Get_Filename() const noexcept;
        void Seek_Beg();
        void Calculate_Valid_Numbers(std::function<bool(T)> valid_fce, config::TThread_Config thread_config);

        template<class E>
        friend std::ostream& operator<<(std::ostream& out, CFile_Reader<E>& file);

    private:
        size_t Calculate_File_Size();

    private:
        std::string m_filename;
        std::ifstream m_file;
        std::mutex m_mtx;
        size_t m_file_size;
        size_t m_total_number_of_elements;
        size_t m_total_number_of_valid_elements;
    };
}