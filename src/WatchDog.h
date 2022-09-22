#pragma once

#include <mutex>
#include <memory>
#include <thread>
#include <list>
#include <unordered_map>
#include <atomic>

namespace kiv_ppr
{
    class CWatch_Dog
    {
        using Time_t = std::chrono::time_point<std::chrono::system_clock>;

    public:
        CWatch_Dog(double interval_ms, size_t maximum_number_of_threads);
        ~CWatch_Dog();

        bool Kick(const std::thread::id& thread_id = std::this_thread::get_id());
        bool Remove(const std::thread::id& thread_id = std::this_thread::get_id());
        size_t Get_Number_Of_Active_Threads() const noexcept;

    private:
        bool Is_Expired(std::thread::id &expired_thread_id);
        auto Get_Next_Expire_Time();
        void Run();

    private:
        std::chrono::duration<double> m_interval_ms;
        size_t m_maximum_number_of_threads;
        size_t m_number_of_threads;
        std::atomic<bool> m_watch_dog_thread_enabled;
        std::thread m_watch_dog_thread;
        std::mutex m_mtx;
        std::list<std::pair<std::thread::id, Time_t>> m_thread_queue;
        std::unordered_map<std::thread::id, decltype(m_thread_queue)::iterator> m_thread_iterators;
    };
}