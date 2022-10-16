#include <iostream>
#include <iomanip>

#include "FileReader.h"
#include "../Config.h"

namespace kiv_ppr
{
    template<typename T>
    CFile_Reader<T>::CFile_Reader(const std::string& filename)
        : m_filename(filename),
          m_number_of_read_elements(0)
    {
        m_file = std::ifstream(filename, std::ios::in | std::ios::binary);

        if (m_file.is_open())
        {
            m_file_size = Calculate_File_Size();
            m_number_of_elements = m_file_size / sizeof(T);
        }
    }

    template<typename T>
    CFile_Reader<T>::~CFile_Reader()
    {
        if (m_file.is_open())
        {
            m_file.close();
        }
    }

    template<typename T>
    bool CFile_Reader<T>::Is_Open() const
    {
        return m_file.is_open();
    }

    template<typename T>
    size_t CFile_Reader<T>::Get_File_Size() const noexcept
    {
        return m_file_size;
    }

    template<typename T>
    size_t CFile_Reader<T>::Get_Number_Of_Elements() const noexcept
    {
        return m_number_of_elements;
    }

    template<typename T>
    std::string CFile_Reader<T>::Get_Filename() const noexcept
    {
        return m_filename;
    }

    template<typename T>
    void CFile_Reader<T>::Seek_Beg()
    {
        m_number_of_read_elements = 0;
        m_file.clear();
        m_file.seekg(0, std::ios::beg);
    }

    template<typename T>
    typename CFile_Reader<T>::TData_Block CFile_Reader<T>::Read_Data(size_t number_of_elements)
    {
        const std::lock_guard<std::mutex> lock(m_mtx);
        if (m_number_of_read_elements + number_of_elements > m_number_of_elements)
        {
            number_of_elements = m_number_of_elements - m_number_of_read_elements;
            if (number_of_elements == 0)
            {
                return { NRead_Status::EOF_, 0, nullptr };
            }
        }
        m_number_of_read_elements += number_of_elements;
        auto buffer = std::shared_ptr<T[]>(new(std::nothrow) T[number_of_elements]);
        if (nullptr == buffer)
        {
            return { NRead_Status::Error, 0, nullptr };
        }
        m_file.read(reinterpret_cast<char*>(buffer.get()), number_of_elements * sizeof(T));
        return { NRead_Status::OK, static_cast<long>(number_of_elements), buffer };
    }

    template<class E>
    std::ostream& operator<<(std::ostream& out, CFile_Reader<E>& file)
    {
        static constexpr uint32_t NUMBER_OF_ELEMENTS_PER_READ = 1;

        if (!file.Is_Open())
        {
            std::cerr << "File has not been opened. Exiting.." << std::endl;
            exit(1);
        }

        file.Seek_Beg();
        while (true)
        {
            const auto [status, count, data] = file.Read_Data(NUMBER_OF_ELEMENTS_PER_READ);
            switch (status)
            {
                case kiv_ppr::CFile_Reader<E>::NRead_Status::OK:
                    for (auto i = 0; i < count; ++i)
                    {
                        out << std::setprecision(kiv_ppr::config::Double_Precision) << data[i] << " ";
                    }
                    break;

                case kiv_ppr::CFile_Reader<E>::NRead_Status::EOF_:
                    return out;

                case kiv_ppr::CFile_Reader<E>::NRead_Status::Error: [[fallthrough]];
                default:
                    std::cerr << "Error occurred printing out the contents of the input file. Exiting..." << std::endl;
                    exit(1);
            }
        }
    }

    template<typename T>
    size_t CFile_Reader<T>::Calculate_File_Size()
    {
        m_file.seekg(0, std::ios::end);
        const auto size = m_file.tellg();
        m_file.seekg(0, std::ios::beg);
        return size;
    }

    template class CFile_Reader<double>;
    template std::ostream& operator<<(std::ostream& out, CFile_Reader<double>& file);
}