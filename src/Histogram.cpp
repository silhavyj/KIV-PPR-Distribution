#include <vector>
#include <future>

#include "Histogram.h"

namespace kiv_ppr
{
    template<class T>
    Histogram<T>::Histogram(uint32_t number_of_slots, T min, T max)
            : m_number_of_slots(number_of_slots),
              m_slots(number_of_slots),
              m_min(min),
              m_max(max)
    {

    }

    template<class T>
    void Histogram<T>::Add(T value)
    {
        static auto slot_size = (m_max - m_min) / m_number_of_slots;
        uint32_t slot_id = (value - m_min) / slot_size;
        ++m_slots[slot_id];
    }

    template<class T>
    uint32_t Histogram<T>::Get_Size() const noexcept
    {
        return m_number_of_slots;
    }

    template<class T>
    std::size_t& Histogram<T>::operator[](uint32_t index)
    {
        return m_slots[index % Get_Size()];
    }

    template<class T>
    void Histogram<T>::operator+=(Histogram<T>& other)
    {
        uint32_t size = std::min(Get_Size(), other.Get_Size());
        for (uint32_t i = 0; i < size; ++i)
        {
            (*this)[i] += other[i];
        }
    }

    template<class T>
    [[nodiscard]] Histogram<T> Histogram<T>::Generate_Histogram(File_Reader<T>& file, uint32_t number_of_slots, T min, T max, uint32_t number_of_threads)
    {
        Histogram<T> histogram(number_of_slots, min, max);
        file.Seek_Beg();

        std::vector<std::future<Histogram<T>>> workers(number_of_threads);
        for (uint32_t i = 0; i < number_of_threads; ++i)
        {
            workers[i] = std::async(std::launch::async, [&file, &number_of_slots, &min, &max]() {
                Histogram<T> sub_histogram(number_of_slots, min, max);
                while (true)
                {
                    const auto [status, count, data] = file.Read_Data(10);
                    switch (status)
                    {
                        case kiv_ppr::File_Reader<T>::Status::OK:
                            for (std::size_t i = 0; i < count; ++i)
                            {
                                sub_histogram.Add(data[i]);
                            }
                            break;
                        case File_Reader<T>::Status::ERROR: [[fallthrough]];
                        case File_Reader<T>::Status::EOF_:
                            return sub_histogram;
                    }
                }
            });
        }
        for (auto& worker : workers)
        {
            auto sub_histogram = worker.get();
            histogram += sub_histogram;
        }
        return histogram;
    }

    template<class E>
    std::ostream& operator<<(std::ostream& out, Histogram<E>& histogram)
    {
        uint32_t size = histogram.Get_Size();
        for (uint32_t i = 0; i < size; ++i)
        {
            out << histogram[i] << " ";
        }
        return out;
    }

    template class Histogram<double>;
    template std::ostream& operator<<(std::ostream& out, Histogram<double>& histogram);
}