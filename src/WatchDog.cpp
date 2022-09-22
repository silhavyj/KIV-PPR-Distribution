#include <iostream>

#include "WatchDog.h"

namespace kiv_ppr
{
    CWatch_Dog::CWatch_Dog(double interval_ms, size_t maximum_number_of_threads)
        : m_interval_ms(interval_ms),
          m_maximum_number_of_threads(maximum_number_of_threads),
          m_number_of_threads{},
          m_watch_dog_thread_enabled{true},
          m_reached_max_number_of_clients{false}
    {
        m_watch_dog_thread = std::thread(&CWatch_Dog::Run, this);
    }

    CWatch_Dog::~CWatch_Dog()
    {
        m_watch_dog_thread_enabled = false;
        m_watch_dog_thread.join();
    }

    bool CWatch_Dog::Reached_Maximum_Number_Of_Clients() const noexcept
    {
        return m_reached_max_number_of_clients;
    }

    bool CWatch_Dog::Kick(const std::thread::id& thread_id)
    {
        const std::lock_guard<std::mutex> lock(m_mtx);

        if (m_thread_iterators.count(thread_id))
        {
            m_thread_queue.erase(m_thread_iterators[thread_id]);
            m_thread_iterators.erase(thread_id);
            --m_number_of_threads;
        }
        else if (m_number_of_threads == m_maximum_number_of_threads)
        {
            return false;
        }

        auto expired_time = std::chrono::system_clock::now();
        expired_time += std::chrono::duration_cast<std::chrono::milliseconds>(m_interval_ms);

        m_thread_queue.emplace_front(thread_id, expired_time);
        m_thread_iterators[thread_id] = m_thread_queue.begin();
        ++m_number_of_threads;

        if (m_number_of_threads == m_maximum_number_of_threads)
        {
            m_reached_max_number_of_clients = true;
        }

        std::cout << "Kicked by " << thread_id << "\n";

        return true;
    }

    bool CWatch_Dog::Remove(const std::thread::id& thread_id)
    {
        const std::lock_guard<std::mutex> lock(m_mtx);
        if (m_thread_iterators.count(thread_id))
        {
            m_thread_queue.erase(m_thread_iterators[thread_id]);
            m_thread_iterators.erase(thread_id);
            --m_number_of_threads;
            return true;
        }
        return false;
    }

    bool CWatch_Dog::Is_Expired(std::thread::id& expired_thread_id)
    {
        const std::lock_guard<std::mutex> lock(m_mtx);
        const auto [id, time] = m_thread_queue.back();

        if (m_thread_queue.size() && time < std::chrono::system_clock::now())
        {
            expired_thread_id = id;
            return true;
        }
        return false;
    }

    auto CWatch_Dog::Get_Next_Expire_Time()
    {
        const std::lock_guard<std::mutex> lock(m_mtx);
        if (m_thread_queue.size())
        {
            const auto [id, time] = m_thread_queue.back();
            return time;
        }
        auto expire_time = std::chrono::system_clock::now();
        expire_time += std::chrono::duration_cast<std::chrono::milliseconds>(m_interval_ms);
        return expire_time;
    }

    size_t CWatch_Dog::Get_Number_Of_Active_Threads() const noexcept
    {
        return m_number_of_threads;
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
            // std::this_thread::sleep_until(Get_Next_Expire_Time());
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}