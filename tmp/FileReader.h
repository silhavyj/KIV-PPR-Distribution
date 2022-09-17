#pragma once

#include <mutex>
#include <memory>
#include <fstream>
#include <string>

namespace kiv_ppr
{
    template<class T>
    class File_Reader
    {
    public:
        enum class Flag
        {
            OK,
            EOF_,
            ERROR
        };

        struct Data_Block
        {
            Flag flag;
            std::size_t count;
            std::shared_ptr<T[]> data;         
        };

    public:
        File_Reader(const std::string& filename, std::size_t block_size);
        ~File_Reader();

        Data_Block Read_Data();
        [[nodiscard]] bool Is_Open() const noexcept;
        [[nodiscard]] std::size_t Get_Size() const noexcept;
        [[nodiscard]] std::size_t Get_Number_Of_Elements() const noexcept;
        void Seek_Beg();

        template<class E>
        friend std::ostream& operator<<(std::ostream& out, File_Reader<E>& file);

    private:
        [[nodiscard]] bool Is_EOF() const noexcept;
        [[nodiscard]] std::size_t Get_Number_Of_Elements_To_Read() const noexcept;
        [[nodiscard]] std::size_t Calculate_File_Size() noexcept;

    private:
        std::size_t m_elements_per_read;
        std::mutex m_file_mtx;
        std::ifstream m_file;
        std::size_t m_file_size;
        std::size_t m_max_read_count;
        std::size_t m_read_count;
        std::size_t m_number_of_trailing_elements;
        std::size_t m_total_number_of_elements;
    };
}