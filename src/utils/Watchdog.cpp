#include "Watchdog.h"

namespace kiv_ppr
{
    CWatchdog::CWatchdog(double interval_sec)
        : m_interval_sec(interval_sec),
          m_enabled{true},
          m_counter(0)
    {
        m_watchdog_thread = std::thread(&CWatchdog::Run, this);
    }

    CWatchdog::~CWatchdog()
    {
        Stop();
        m_watchdog_thread.join();
    }

    void CWatchdog::Stop()
    {
        m_enabled = false;
    }

    size_t CWatchdog::Get_Counter_Value() const
    {
        return m_counter;
    }

    void CWatchdog::Kick(size_t value)
    {
        const std::lock_guard<std::mutex> lock(m_mtx);
        m_counter += value;
    }

    void CWatchdog::Run()
    {
        size_t current_value;
        size_t previous_value = m_counter;
        bool first_check = true;

        while (m_enabled)
        {
            current_value = m_counter;
            if (!first_check && previous_value == current_value)
            {
                std::exit(5);
            }
            first_check = false;
            current_value = previous_value;
            std::this_thread::sleep_for(m_interval_sec);
        }
    }
}