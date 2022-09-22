#include <iomanip>
#include <iostream>
#include <future>

#include "FileReader.h"

namespace kiv_ppr
{
    template<class T>
    CFile_Reader<T>::CFile_Reader(const std::string &filename)
        : m_total_number_of_valid_elements{}
    {
        m_file = std::ifstream(filename, std::ios::in | std::ios::binary);

        if (m_file.is_open())
        {
            m_file_size = Calculate_File_Size();
            m_total_number_of_elements = m_file_size / sizeof(T);
        }
    }

    template<class T>
    CFile_Reader<T>::~CFile_Reader()
    {
        if (m_file && m_file.is_open())
        {
            m_file.close();
        }
    }

    template<class T>
    bool CFile_Reader<T>::Is_Open() const
    {
        return m_file.is_open();
    }

    template<class T>
    typename CFile_Reader<T>::TData_Block CFile_Reader<T>::Read_Data(size_t number_of_elements)
    {
        const std::lock_guard<std::mutex> lock(m_mtx);
        if (m_file.eof())
        {
            return { NStatus::EOF_, 0, nullptr };
        }

        auto buffer = std::shared_ptr<T[]>(new(std::nothrow) T[number_of_elements]);
        if (nullptr == buffer)
        {
            return { NStatus::ERROR, 0, nullptr };
        }

        m_file.read(reinterpret_cast<char*>(buffer.get()), number_of_elements * sizeof(T));
        if (0 == m_file.gcount())
        {
            return { NStatus::EOF_, 0, nullptr };
        }
        return { NStatus::OK, static_cast<size_t>(m_file.gcount()) / sizeof(T), buffer };
    }

    template<class T>
    void CFile_Reader<T>::Seek_Beg()
    {
        m_file.clear();
        m_file.seekg(0, std::ios::beg);
    }

    template<class T>
    size_t CFile_Reader<T>::Calculate_File_Size()
    {
        m_file.seekg(0, std::ios::end);
        const auto size = m_file.tellg();
        m_file.seekg(0, std::ios::beg);
        return size;
    }

    template<class T>
    [[nodiscard]] size_t CFile_Reader<T>::Get_Total_Number_Of_Elements() const noexcept
    {
        return m_total_number_of_elements;
    }

    template<class T>
    [[nodiscard]] size_t CFile_Reader<T>::Get_Total_Number_Of_Valid_Elements() const noexcept
    {
        return m_total_number_of_valid_elements;
    }

    template<class T>
    void CFile_Reader<T>::Calculate_Valid_Numbers(std::function<bool(T)> valid_fce, config::TThread_Config thread_config)
    {
        Seek_Beg();
        std::vector<std::future<size_t>> workers(thread_config.number_of_threads);
        for (uint32_t i = 0; i < thread_config.number_of_threads; ++i)
        {
            workers[i] = std::async(std::launch::async, [&]() {
                size_t local_count = 0;
                while (true)
                {
                    const auto [status, count, data] = Read_Data(thread_config.number_of_elements_per_file_read);
                    switch (status)
                    {
                        case kiv_ppr::CFile_Reader<T>::NStatus::OK:
                            for (size_t i = 0; i < count; ++i)
                            {
                                if (valid_fce(data[i]))
                                {
                                    ++local_count;
                                }
                            }
                            break;
                        case NStatus::ERROR:
                            std::cerr << L"Error occurred while calculating the number of valid elements in the input file. Exiting...\n";
                            exit(__LINE__);
                        case NStatus::EOF_:
                            return local_count;
                    }
                }
            });
        }
        for (auto& worker : workers)
        {
            const auto local_count = worker.get();
            m_total_number_of_valid_elements += local_count;
        }
    }

    template<class E>
    std::ostream& operator<<(std::ostream& out, CFile_Reader<E>& file) {
        file.Seek_Beg();
        while (true)
        {
            const auto [status, count, data] = file.Read_Data(1);
            switch (status)
            {
                case kiv_ppr::CFile_Reader<E>::NStatus::OK:
                    for (size_t i = 0; i < count; ++i)
                    {
                        out << std::setprecision(9) << data[i] << " ";
                    }
                    break;
                case CFile_Reader<E>::NStatus::EOF_:
                    return out;
                case CFile_Reader<E>::NStatus::ERROR:
                    std::cerr << L"Error occurred printing out the contents of the input file. Exiting...\n";
                    exit(__LINE__);
            }
        }
    }

    template class CFile_Reader<double>;
    template std::ostream& operator<<(std::ostream& out, CFile_Reader<double>& file);
}