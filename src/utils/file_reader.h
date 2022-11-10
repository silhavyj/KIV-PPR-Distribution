#pragma once

#include <cstdint>
#include <memory>
#include <fstream>
#include <mutex>

namespace kiv_ppr
{
    /// \author Jakub Silhavy 
    /// \tparam T Data type the bytes of the input file will be treated as.
    ///
    /// This class provides a thread-safe functions for reading a binary file.
    /// It is used by worker threads when processing the input file.
    template<typename T>
    class CFile_Reader
    {
    public:
        /// Result of reading a data block from the input file.
        enum class [[nodiscard]] NRead_Status : uint8_t
        {
            OK,    ///< All good
            Error, ///< An error has ocurred
            EOF_   ///< End of file has been reached
        };

        /// Data block read from the input file.
        struct TData_Block
        {
            NRead_Status status;       ///< Read status (OK, Error, EOF_)
            size_t count;              ///< Number of values read from the file
            std::shared_ptr<T[]> data; ///< Data itself (heap allocation)
        };

    public:
        /// Creates an instance of the class. 
        /// \param filename Path to the input file 
        explicit CFile_Reader(const std::string& filename);

        /// Default destructor.
        ~CFile_Reader() = default;

        /// Returns whether the input file is open or not.
        /// \return true, if the input file is open, false otherwise.
        [[nodiscard]] bool Is_Open() const;

        /// Returns the size of the input file.
        /// \return Size of the input file in bytes.
        [[nodiscard]] size_t Get_File_Size() const noexcept;

        /// Returns the number of elements in the input file.
        /// This values is given by the datatype T.
        /// \return Number of elements in the input file.
        [[nodiscard]] size_t Get_Number_Of_Elements() const noexcept;
        
        /// Returns the input file name.
        /// \return Name of the input file.
        [[nodiscard]] std::string Get_Filename() const noexcept;

        /// Seeks to the beginning of the input file.
        void Seek_Beg();

        /// Reads a block of data from the input file.
        /// This method is periodically called from the worker threads.
        /// \param number_of_elements Number of elements to be read from the input file.
        /// \return Block of data read from the input file.
        [[nodiscard]] TData_Block Read_Data(size_t number_of_elements);

        /// Prints out the contents of the input file to an output stream.
        /// @tparam Datatype associated with the file reader.
        /// @param out Output stream.
        /// @param file File reader.
        /// @return Output stream.
        template<class E>
        friend std::ostream& operator<<(std::ostream& out, CFile_Reader<E>& file);

    private:
        /// Calculates the size of the input file. 
        /// @return Size of the input file.
        [[nodiscard]] size_t Calculate_File_Size();

    private:
        std::string m_filename;                ///< Path to the input file
        std::ifstream m_file;                  ///< Input stream (reading data from a file)
        std::mutex m_mtx;                      ///< Mutex used when reading from the input file
        size_t m_file_size;                    ///< Size of the input file
        std::size_t m_number_of_elements;      ///< Total number of elements in the input file
        std::size_t m_number_of_read_elements; ///< Number of elements read from the file since the last Seek_Beg()
    };
}

// EOF