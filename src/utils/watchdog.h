#pragma once

#include <atomic>
#include <cstddef>
#include <mutex>
#include <thread>

namespace kiv_ppr
{
    class CWatchdog
    {
    public:
        explicit CWatchdog(double interval_sec);
        ~CWatchdog();

        void Stop();
        void Kick(size_t value);
        [[nodiscard]] size_t Get_Counter_Value() const;

    private:
        void Run();

    private:
        std::chrono::duration<double> m_interval_sec;
        std::atomic<bool> m_enabled;
        size_t m_counter;
        std::mutex m_mtx;
        std::thread m_watchdog_thread;
    };
}