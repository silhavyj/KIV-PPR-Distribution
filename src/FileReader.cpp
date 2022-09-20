#include <iomanip>
#include <iostream>

#include "FileReader.h"

namespace kiv_ppr
{
    template<class T>
    File_Reader<T>::File_Reader(const std::string &filename)
    {
        m_file = std::ifstream(filename, std::ios::in | std::ios::binary);

        if (m_file.is_open())
        {
            m_file_size = Calculate_File_Size();
            m_total_number_of_elements = m_file_size / sizeof(T);
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
    bool File_Reader<T>::Is_Open() const
    {
        return m_file.is_open();
    }

    template<class T>
    typename File_Reader<T>::Data_Block File_Reader<T>::Read_Data(std::size_t number_of_elements)
    {
        const std::lock_guard<std::mutex> lock(m_mtx);
        if (m_file.eof())
        {
            return { Status::EOF_, 0, nullptr };
        }

        auto buffer = std::shared_ptr<T[]>(new(std::nothrow) T[number_of_elements]);
        if (nullptr == buffer)
        {
            return { Status::ERROR, 0, nullptr };
        }

        m_file.read(reinterpret_cast<char*>(buffer.get()), number_of_elements * sizeof(T));
        if (0 == m_file.gcount())
        {
            return { Status::EOF_, 0, nullptr };
        }
        return { Status::OK, static_cast<std::size_t>(m_file.gcount()) / sizeof(T), buffer };
    }

    template<class T>
    void File_Reader<T>::Seek_Beg()
    {
        m_file.clear();
        m_file.seekg(0, std::ios::beg);
    }

    template<class T>
    std::size_t File_Reader<T>::Calculate_File_Size()
    {
        m_file.seekg(0, std::ios::end);
        const auto size = m_file.tellg();
        m_file.seekg(0, std::ios::beg);
        return size;
    }

    template<class T>
    [[nodiscard]] std::size_t File_Reader<T>::Get_Total_Number_Of_Elements() const noexcept
    {
        return m_total_number_of_elements;
    }

    template<class E>
    std::ostream& operator<<(std::ostream& out, File_Reader<E>& file) {
        file.Seek_Beg();
        while (true)
        {
            const auto [status, count, data] = file.Read_Data(1);
            switch (status)
            {
                case kiv_ppr::File_Reader<E>::Status::OK:
                    for (std::size_t i = 0; i < count; ++i)
                    {
                        out << std::setprecision(9) << data[i] << " ";
                    }
                    break;
                case File_Reader<E>::Status::EOF_:
                    return out;
                case File_Reader<E>::Status::ERROR:
                    std::cerr << "Error has occurred while printing out the contents of the input file\n";
                    return out;
            }
        }
    }

    template class File_Reader<double>;
    template std::ostream& operator<<(std::ostream& out, File_Reader<double>& file);
}