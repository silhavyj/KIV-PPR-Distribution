#include <iostream>

#include "WatchDog.h"

namespace kiv_ppr
{
    CWatch_Dog::CWatch_Dog(double interval_sec)
        : m_interval_sec(interval_sec),
          m_watch_dog_thread_enabled{true}
    {
        m_watch_dog_thread = std::thread(&CWatch_Dog::Run, this);
    }

    CWatch_Dog::~CWatch_Dog()
    {
        m_watch_dog_thread_enabled = false;
        m_watch_dog_thread.join();
    }

    bool CWatch_Dog::Register(const Thread_ID_t& thread_id)
    {
        const std::lock_guard<std::mutex> lock(m_mtx);
        if (m_registered_threads.count(thread_id))
        {
            return false;
        }
        m_registered_threads.insert(thread_id);
        return true;
    }

    bool CWatch_Dog::Kick(const Thread_ID_t& thread_id)
    {
        const std::lock_guard<std::mutex> lock(m_mtx);
        if (!m_registered_threads.count(thread_id))
        {
            return false;
        }

        if (m_queue_iterators.count(thread_id))
        {
            m_thread_queue.erase(m_queue_iterators[thread_id]);
            m_queue_iterators.erase(thread_id);
        }

        Time_t expired_time = Get_Expired_Time();
        m_thread_queue.emplace_front(thread_id, expired_time);
        m_queue_iterators[thread_id] = m_thread_queue.begin();

        std::cout << "Kicked by " << thread_id << "\n";
        return true;
    }

    bool CWatch_Dog::Unregister(const Thread_ID_t& thread_id)
    {
        const std::lock_guard<std::mutex> lock(m_mtx);
        if (m_registered_threads.count(thread_id))
        {
            m_thread_queue.erase(m_queue_iterators[thread_id]);
            m_queue_iterators.erase(thread_id);
            m_registered_threads.erase(thread_id);
            return true;
        }
        return false;
    }

    [[nodiscard]] size_t CWatch_Dog::Get_Number_Of_Registered_Threads()
    {
        return m_registered_threads.size();
    }

    [[nodiscard]] const CWatch_Dog::Time_t CWatch_Dog::Get_Expired_Time() const
    {
        auto expired_time = std::chrono::system_clock::now();
        expired_time += std::chrono::duration_cast<std::chrono::milliseconds>(m_interval_sec);
        return expired_time;
    }

    [[nodiscard]] const CWatch_Dog::Time_t CWatch_Dog::Get_Next_Expire_Time()
    {
        const std::lock_guard<std::mutex> lock(m_mtx);
        if (Get_Number_Of_Registered_Threads())
        {
            const auto [thread_id, time] = m_thread_queue.back();
            return time;
        }
        const Time_t expired_time = Get_Expired_Time();
        return expired_time;
    }

    [[nodiscard]] bool CWatch_Dog::Is_Expired(std::thread::id& expired_thread_id)
    {
        const std::lock_guard<std::mutex> lock(m_mtx);
        if (Get_Number_Of_Registered_Threads())
        {
            const auto [thread_id, time] = m_thread_queue.back();
            if (time < std::chrono::system_clock::now())
            {
                expired_thread_id = thread_id;
                return true;
            }
        }
        return false;
    }

    void CWatch_Dog::Run()
    {
        Thread_ID_t thread_id;
        while (m_watch_dog_thread_enabled)
        {
            if (Is_Expired(thread_id))
            {
                std::cerr << "Thread " << thread_id << " seems to have not been active enough. Exiting...";
                exit(1);
            }
            std::this_thread::sleep_until(Get_Next_Expire_Time());
        }
    }
}