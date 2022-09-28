#pragma once

#include <cstdint>
#include <memory>
#include <fstream>
#include <mutex>

namespace kiv_ppr
{
    template<typename T>
    class CFile_Reader
    {
    public:
        enum class [[nodiscard]] NRead_Status : uint8_t
        {
            OK,
            ERROR,
            EOF_
        };

        struct TData_Block
        {
            NRead_Status status;
            size_t count;
            std::shared_ptr<T[]> data;
        };

    public:
        explicit CFile_Reader(const std::string& filename);
        ~CFile_Reader();

        [[nodiscard]] bool Is_Open() const;
        [[nodiscard]] size_t Get_File_Size() const noexcept;
        [[nodiscard]] std::string Get_Filename() const noexcept;

        void Seek_Beg();
        [[nodiscard]] TData_Block Read_Data(size_t number_of_elements);

        template<class E>
        friend std::ostream& operator<<(std::ostream& out, CFile_Reader<E>& file);

    private:
        size_t Calculate_File_Size();

    private:
        std::string m_filename;
        std::ifstream m_file;
        std::mutex m_mtx;
        size_t m_file_size;
    };
}