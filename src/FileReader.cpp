#include <spdlog/spdlog.h>

#include "FileReader.h"

namespace kiv_ppr
{
    template<class T>
    File_Reader<T>::File_Reader(const std::string& filename, std::size_t elements_per_read)
        : m_elements_per_read(elements_per_read), m_read_count{0}
    {
        m_file = std::ifstream(filename, std::ios::binary);

        if (m_file.is_open())
        {
            m_file_size = Calculate_File_Size();
            m_total_number_of_elements = m_file_size / sizeof(T);
            m_max_read_count = m_file_size / (m_elements_per_read * sizeof(T));
            m_number_of_trailing_elements = m_total_number_of_elements % m_elements_per_read;
        }
    }
    
    template<class T>
    File_Reader<T>::~File_Reader()
    {
        if (m_file && m_file.is_open())
        {
            m_file.close();
        }
    }

    template<class T>
    typename File_Reader<T>::Data_Block File_Reader<T>::Read_Data()
    {
        const std::lock_guard<std::mutex> lock(m_file_mtx);
        if (Is_EOF())
        {
            return { Flag::EOF_, 0, nullptr };
        }

        std::size_t number_of_elements = Get_Number_Of_Elements_To_Read();
        auto buffer = std::shared_ptr<T[]>(new(std::nothrow) T[number_of_elements]);

        if (nullptr == buffer)
        {
            return { Flag::ERROR, 0, nullptr };
        }

        m_file.read(reinterpret_cast<char*>(buffer.get()), number_of_elements * sizeof(T));
        ++m_read_count;

        return { Flag::OK, number_of_elements, buffer };
    }

    template<class T>
    inline std::size_t File_Reader<T>::Get_Number_Of_Elements_To_Read() const noexcept
    {
        if (m_read_count == m_max_read_count)
        {
           return m_number_of_trailing_elements;
        }
        else
        {
            return m_elements_per_read;
        }
    }

    template<class T>
    inline bool File_Reader<T>::Is_Open() const noexcept
    {
        return m_file.is_open();
    }

    template<class T>
    inline bool File_Reader<T>::Is_EOF() const noexcept
    {
        return m_read_count >= (m_max_read_count + 1);
    }

    template<class T>
    std::size_t File_Reader<T>::Get_Size() const noexcept
    {
        return m_file_size;
    }

    template<class T>
    std::size_t File_Reader<T>::Calculate_File_Size() noexcept
    {
        m_file.seekg(0, std::ios::end);
        const auto size = m_file.tellg();
        m_file.seekg(0, std::ios::beg);
        return size;
    }

    template<class T>
    std::size_t File_Reader<T>::Get_Number_Of_Elements() const noexcept
    {
        return m_total_number_of_elements;
    }

    template<class T>
    void File_Reader<T>::Seek_Beg()
    {
        m_file.seekg(0, std::ios::end);
        m_read_count = 0;
    }

    template class File_Reader<double>;
}
