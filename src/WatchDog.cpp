#include <iostream>

#include "WatchDog.h"

namespace kiv_ppr
{
    CWatch_Dog::CWatch_Dog(double interval_ms, size_t maximum_number_of_threads)
        : m_interval_ms(interval_ms),
          m_maximum_number_of_threads(maximum_number_of_threads),
          m_number_of_threads{},
          m_watch_dog_thread_enabled{true}
    {
        m_watch_dog_thread = std::thread(&CWatch_Dog::Run, this);
    }

    CWatch_Dog::~CWatch_Dog()
    {
        m_watch_dog_thread_enabled = false;
        m_watch_dog_thread.join();
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
        expired_time += std::chrono::duration_cast<std::chrono::milliseconds>(m_interval_ms);
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
            m_list.erase(m_hash_map[thread_id]);
            m_hash_map.erase(thread_id);
            --m_number_of_threads;
            return true;
        }
        return false;
    }

    bool CWatch_Dog::Is_Expired(std::thread::id &expired_thread_id)
    {
        const std::lock_guard<std::mutex> lock(m_mtx);
        const auto [id, time] = m_list.back();
        if (m_list.size() && time < std::chrono::system_clock::now())
        {
            expired_thread_id = id;
            return true;
        }
        return false;
    }

    auto CWatch_Dog::Get_Next_Expire_Time()
    {
        const std::lock_guard<std::mutex> lock(m_mtx);
        auto expire_time = std::chrono::system_clock::now();
        expire_time += std::chrono::duration_cast<std::chrono::milliseconds>(m_interval_ms);

        if (m_list.size())
        {
            const auto [id, time] = m_list.back();
            return time;
        }
        return expire_time;
    }

    void CWatch_Dog::Run()
    {
        while (m_watch_dog_thread_enabled)
        {
            std::thread::id expired_thread_id{};
            if (Is_Expired(expired_thread_id))
            {
                std::cerr << "Thread " << expired_thread_id << " seems to have not been active enough. Exiting...";
                exit(1);
            }
            std::this_thread::sleep_until(Get_Next_Expire_Time());
        }
    }
}