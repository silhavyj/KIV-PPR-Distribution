#pragma once

#include <mutex>
#include <memory>
#include <thread>
#include <list>
#include <unordered_map>

namespace kiv_ppr
{
    class CWatch_Dog
    {
        using Time_t = std::chrono::time_point<std::chrono::system_clock>;

    public:
        CWatch_Dog(double interval, size_t maximum_number_of_threads);
        ~CWatch_Dog();

        bool Kick(const std::thread::id& thread_id = std::this_thread::get_id());
        bool Remove(const std::thread::id& thread_id = std::this_thread::get_id());

    private:
        std::chrono::duration<double> m_interval;
        size_t m_maximum_number_of_threads;
        size_t m_number_of_threads;
        std::mutex m_mtx;
        std::list<std::pair<std::thread::id, Time_t>> m_list;
        std::unordered_map<std::thread::id, decltype(m_list)::iterator> m_hash_map;
    };
}