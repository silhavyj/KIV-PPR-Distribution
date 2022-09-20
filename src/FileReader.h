#pragma once

#include <cstdint>
#include <mutex>
#include <fstream>
#include <string>
#include <memory>
#include <functional>

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
            EOF_
        };

        struct Data_Block
        {
            Status status;
            std::size_t count;
            std::shared_ptr<T[]> data;
        };

    public:
        explicit File_Reader(const std::string& filename);
        ~File_Reader();

        [[nodiscard]] Data_Block Read_Data(std::size_t number_of_elements);
        [[nodiscard]] bool Is_Open() const;
        void Seek_Beg();
        void Calculate_Valid_Numbers(std::function<bool(T)> valid_fce, uint32_t number_of_threads, std::size_t number_of_elements_per_read);
        [[nodiscard]] std::size_t Get_Total_Number_Of_Elements() const noexcept;
        [[nodiscard]] std::size_t Get_Total_Number_Of_Valid_Elements() const noexcept;

        template<class E>
        friend std::ostream& operator<<(std::ostream& out, File_Reader<E>& file);

    private:
        std::size_t Calculate_File_Size();

    private:
        std::ifstream m_file;
        std::mutex m_mtx;
        std::size_t m_file_size;
        std::size_t m_total_number_of_elements;
        std::size_t m_total_number_of_valid_elements;
    };
}