#include <iostream>

#include "watchdog.h"

namespace kiv_ppr
{
    CWatchdog::CWatchdog(double interval_sec) noexcept
        : m_interval_sec(interval_sec),
          m_enabled{false},
          m_counter(0)
    {

    }

    CWatchdog::~CWatchdog()
    {
        // Stop the watchdog
        Stop();
    }

    void CWatchdog::Start()
    {
        const std::lock_guard<std::mutex> lock(m_mtx);

        // Start the watchdog thread (if it has not been started yet).
        if (!m_enabled)
        {
            m_watchdog_thread = std::thread(&CWatchdog::Run, this);
            m_watchdog_thread.detach();
            m_enabled = true;
        }
    }
     
    void CWatchdog::Stop() noexcept
    {
        m_enabled = false;
    }

    size_t CWatchdog::Get_Counter_Value() const noexcept
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
        size_t current_value{};
        size_t previous_value = m_counter;

        while (m_enabled)
        {
            // Go to sleep for n seconds.
            std::this_thread::sleep_for(m_interval_sec);

            // Get the current value of the counter.
            m_mtx.lock();
            current_value = m_counter;
            m_mtx.unlock();

            // If the counter value has not changed, print out a warning message.
            if (m_enabled && previous_value == current_value)
            {
                std::cout << "Warning (watchdog): Program seems to be inactive" << std::endl;
            }

            // Store the last value of the counter.
            previous_value = current_value;
        }
    }
}

// EOF