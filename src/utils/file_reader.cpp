#include <iostream>
#include <iomanip>

#include "file_reader.h"
#include "../config.h"

namespace kiv_ppr
{
    template<typename T>
    CFile_Reader<T>::CFile_Reader(const std::string& filename)
        : m_filename(filename),
          m_number_of_read_elements(0)
    {
        // Open the input file.
        m_file = std::ifstream(filename, std::ios::in | std::ios::binary);

        // If the file is open, calculate its size and
        // the number of elements it in.
        if (m_file.is_open())
        {
            m_file_size = Calculate_File_Size();
            m_number_of_elements = m_file_size / sizeof(T);
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
        // Mutual exclusion
        const std::lock_guard<std::mutex> lock(m_mtx);

        // Check if the requested number of elements does not exceed
        // the total number of elements in the file. If so, read only the remaining
        // elements if there are any.
        if (m_number_of_read_elements + number_of_elements > m_number_of_elements)
        {
            number_of_elements = m_number_of_elements - m_number_of_read_elements;
            if (number_of_elements == 0 || m_number_of_elements < m_number_of_read_elements)
            {
                return { NRead_Status::EOF_, 0, nullptr };
            }
        }

        // Update the total number of elements read from the file so far.
        m_number_of_read_elements += number_of_elements;

        // Create a buffer for the elements to be read from the file.
        auto buffer = std::shared_ptr<T[]>(new(std::nothrow) T[number_of_elements]);
        if (nullptr == buffer)
        {
            return { NRead_Status::Error, 0, nullptr };
        }

        // Read the elements from the input file.
        m_file.read(reinterpret_cast<char*>(buffer.get()), number_of_elements * sizeof(T));

        return { NRead_Status::OK, number_of_elements, buffer };
    }

    template<class E>
    std::ostream& operator<<(std::ostream& out, CFile_Reader<E>& file)
    {
        // Read the elements from the input file one by one.
        static constexpr uint32_t NUMBER_OF_ELEMENTS_PER_READ = 1;

        if (!file.Is_Open())
        {
            std::cout << "File has not been opened. Exiting.." << std::endl;
            exit(1);
        }

        // Seek to the beginning of the file.
        file.Seek_Beg();

        while (true)
        {
            // Read one element from the input file.
            const auto [status, count, data] = file.Read_Data(NUMBER_OF_ELEMENTS_PER_READ);

            switch (status)
            {
                // Print the element out to the stream.
                case kiv_ppr::CFile_Reader<E>::NRead_Status::OK:
                    for (auto i = 0; i < count; ++i)
                    {
                        out << std::setprecision(kiv_ppr::config::Double_Precision) << data[i] << " ";
                    }
                    break;

                // The end of file has been reached.
                case kiv_ppr::CFile_Reader<E>::NRead_Status::EOF_:
                    return out;

                // An error has ocurred.
                case kiv_ppr::CFile_Reader<E>::NRead_Status::Error: [[fallthrough]];
                default:
                    std::cout << "Error occurred printing out the contents of the input file. Exiting..." << std::endl;
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

    // Create a file reader with a double datatype.
    template class CFile_Reader<double>;
    template std::ostream& operator<<(std::ostream& out, CFile_Reader<double>& file);
}

// EOF