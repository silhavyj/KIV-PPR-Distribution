#pragma once

#include <mutex>
#include <thread>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <atomic>

namespace kiv_ppr
{
    class CWatch_Dog
    {
    public:
        using Time_t = std::chrono::time_point<std::chrono::system_clock>;
        using Thread_ID_t = std::thread::id;

    public:
        explicit CWatch_Dog(double interval_sec);
        ~CWatch_Dog();

        bool Register(const Thread_ID_t& thread_id = std::this_thread::get_id());
        bool Kick(const Thread_ID_t& thread_id = std::this_thread::get_id());
        bool Unregister(const Thread_ID_t& thread_id = std::this_thread::get_id());
        [[nodiscard]] size_t Get_Number_Of_Registered_Threads();

    private:
        [[nodiscard]] Time_t Get_Expired_Time() const;
        [[nodiscard]] Time_t Get_Next_Expire_Time();
        [[nodiscard]] bool Is_Expired(std::thread::id& expired_thread_id);
        void Run();

    private:
        std::chrono::duration<double> m_interval_sec;
        std::atomic<bool> m_watch_dog_thread_enabled;
        std::thread m_watch_dog_thread;
        std::mutex m_mtx;
        std::unordered_set<Thread_ID_t> m_registered_threads;
        std::list<std::pair<Thread_ID_t, Time_t>> m_thread_queue;
        std::unordered_map<Thread_ID_t, decltype(m_thread_queue)::iterator> m_queue_iterators;
    };
}