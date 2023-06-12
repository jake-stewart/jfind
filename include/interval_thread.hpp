#ifndef INTERVAL_THREAD_HPP
#define INTERVAL_THREAD_HPP

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>
#include "logger.hpp"

class IntervalThread
{
public:
    void setInterval(std::chrono::duration<long double, std::milli> interval);
    void start();
    void end();
    bool ticked();
    void restart();

private:
    void threadFunc();

    Logger m_logger = Logger("IntervalThread");
    std::chrono::duration<long double, std::milli> m_interval;
    bool m_active = false;
    bool m_ticked = false;
    std::mutex m_mut;
    std::condition_variable m_cv;
    std::thread m_thread;
};

#endif
