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

        template<class E>
        friend std::ostream& operator<<(std::ostream& out, File_Reader<E>& file);

    private:
        std::size_t m_number_of_elements_per_read;
        std::ifstream m_file;
        std::mutex m_mtx;
    };
}