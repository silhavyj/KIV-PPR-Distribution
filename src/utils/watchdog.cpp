#include <iostream>

#include "watchdog.h"

namespace kiv_ppr
{
    CWatchdog::CWatchdog(double interval_sec) noexcept
        : m_interval_sec(interval_sec),
          m_enabled{false},
          m_counter(0),
          m_init_flag{}
    {

    }

    void CWatchdog::Start()
    {
        // Start the watchdog thread (if it has not been started yet).
        std::call_once(m_init_flag, [&]() {
            m_enabled = true;
            m_watchdog_thread = std::thread(&CWatchdog::Run, this);
        });
    }
     
    void CWatchdog::Stop() noexcept
    {
        m_enabled = false;
        m_watchdog_thread.join();
    }

    size_t CWatchdog::Get_Counter_Value() const noexcept
    {
        return m_counter;
    }

    void CWatchdog::Kick(size_t value)
    {
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
            current_value = m_counter;

            // If the counter value has not changed, print out a warning message.
            if (m_enabled && previous_value == current_value)
            {
                std::cout << "Warning(watchdog) : Program seems to be inactive" << std::endl;
            }

            // Store the last value of the counter.
            previous_value = current_value;
        }
    }
}

// EOF