#pragma once

#include <cstdint>
#include <mutex>
#include <fstream>
#include <string>
#include <memory>

namespace kiv_ppr
{
    template<class T>
    class File_Reader
    {
    public:
        enum class Status : uint8_t
        {
            OK,
            ERROR,
            EMPTY,
            EOF_
        };

        struct Data_Block
        {
            Status status;
            std::size_t count;
            std::shared_ptr<T[]> data;
        };

    public:
        File_Reader(const std::string& filename, std::size_t number_of_elements_per_read);
        ~File_Reader();

        [[nodiscard]] Data_Block Read_Data();
        [[nodiscard]] bool Is_Open() const;
        void Seek_Beg();
        [[nodiscard]] std::size_t Get_Total_Number_Of_Elements() const noexcept;

        template<class E>
        friend std::ostream& operator<<(std::ostream& out, File_Reader<E>& file);

    private:
        std::size_t Calculate_File_Size();

    private:
        std::size_t m_number_of_elements_per_read;
        std::ifstream m_file;
        std::mutex m_mtx;
        std::size_t m_file_size;
        std::size_t m_total_number_of_elements;
    };
}