#include <iostream>

#include "watchdog.h"

namespace kiv_ppr
{
    CWatchdog::CWatchdog(double interval_sec)
        : m_interval_sec(interval_sec),
          m_enabled{false},
          m_counter(0)
    {

    }

    CWatchdog::~CWatchdog()
    {
        Stop();
    }

    void CWatchdog::Start()
    {
        const std::lock_guard<std::mutex> lock(m_mtx);
        if (!m_enabled)
        {
            m_watchdog_thread = std::thread(&CWatchdog::Run, this);
            m_watchdog_thread.detach();
            m_enabled = true;
        }
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

        while (m_enabled)
        {
            std::this_thread::sleep_for(m_interval_sec);

            current_value = m_counter;
            if (m_enabled && previous_value == current_value)
            {
                std::cout << "Watchodog: Program seems to be inactive. Exiting..." << std::endl;
                std::exit(5);
            }
            previous_value = current_value;
        }
    }
}