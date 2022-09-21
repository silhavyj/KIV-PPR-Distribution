#include "WatchDog.h"

namespace kiv_ppr
{
    CWatch_Dog::CWatch_Dog(double interval, size_t maximum_number_of_threads)
        : m_interval(interval),
          m_maximum_number_of_threads(maximum_number_of_threads),
          m_number_of_threads{}
    {

    }

    CWatch_Dog::~CWatch_Dog()
    {

    }

    bool CWatch_Dog::Kick(const std::thread::id &thread_id)
    {
        const std::lock_guard<std::mutex> lock(m_mtx);

        if (m_hash_map.count(thread_id))
        {
            m_list.erase(m_hash_map[thread_id]);
            m_hash_map.erase(thread_id);
            --m_number_of_threads;
        }
        else if (m_number_of_threads == m_maximum_number_of_threads)
        {
            return false;
        }

        auto expired_time = std::chrono::system_clock::now();
        expired_time += std::chrono::duration_cast<std::chrono::milliseconds>(m_interval);
        m_list.emplace_front(thread_id, expired_time);
        m_hash_map[thread_id] = m_list.begin();
        ++m_number_of_threads;

        return true;
    }

    bool CWatch_Dog::Remove(const std::thread::id &thread_id)
    {
        const std::lock_guard<std::mutex> lock(m_mtx);

        if (m_hash_map.count(thread_id))
        {

        }
    }
}