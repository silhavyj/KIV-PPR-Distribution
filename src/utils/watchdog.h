#pragma once

#include <atomic>
#include <cstddef>
#include <mutex>
#include <thread>

namespace kiv_ppr
{
    /// \author Jakub Silhavy
    /// The purpose of this class is to check if the input file
    /// is being processed correctly. If worker threads are not
    /// being scheduled fast enough or they take too much time, 
    /// the watchdog prints out a warning message.
    class CWatchdog
    {
    public:
        /// Creates an instance of the class. 
        /// The watchdog period needs to be set with regards to 
        /// the size of a single data block (the bigger the data block
        /// the more time it takes to read it from the input file).
        /// \param interval_sec Watchdog period [s]
        explicit CWatchdog(double interval_sec) noexcept;

        /// Default destructor.
        ~CWatchdog();

        /// Delete copy constructor.
        CWatchdog(const CWatchdog&) = delete;

        /// Delete copy constructor (R value).
        CWatchdog(CWatchdog&&) = delete;

        /// Delete assignment operator.
        CWatchdog& operator=(const CWatchdog&) = delete;

        /// Delete assignment operator (R value).
        CWatchdog& operator=(const CWatchdog&&) = delete;

        /// Start the watchdog thread. 
        void Start();

        /// Stop the watchdog thread.
        void Stop() noexcept;

        /// Kick the watchdog.
        /// It adds how many values a worker thread has processed to the total sum.
        /// \param value Number of values a worker thread just processed
        void Kick(size_t value);

        /// Returns the total sum (number of values processed by all worker threads). 
        /// \return Number of values processed in total.
        [[nodiscard]] size_t Get_Counter_Value() const noexcept;

    private:
        /// Run function of the watchdog thread.
        void Run();

    private:
        std::chrono::duration<double> m_interval_sec; ///< Watchdog period
        std::atomic<bool> m_enabled;                  ///< Flag indicating if the watchdog thread should be active or dead
        size_t m_counter;                             ///< Total sum (number of values processed by all worker threads)
        std::mutex m_mtx;                             ///< Mutex used when kicking the watchdog
        std::thread m_watchdog_thread;                ///< Watchdog thread
    };
};

// EOF