#include "../include/interval_thread.hpp"

void IntervalThread::setInterval(
    std::chrono::duration<long double, std::milli> interval
) {
    m_interval = interval;
}

void IntervalThread::start() {
    std::unique_lock lock(m_mut);
    if (m_active) {
        return;
    }
    m_active = true;
    m_thread = std::thread(&IntervalThread::threadFunc, this);
}

void IntervalThread::end() {
    {
        std::unique_lock lock(m_mut);
        if (!m_active) {
            return;
        }
        m_active = false;
        m_cv.notify_one();
    }
    m_thread.join();
}

bool IntervalThread::ticked() {
    return m_ticked;
}

void IntervalThread::restart() {
    m_ticked = false;
}

void IntervalThread::threadFunc() {
    std::unique_lock lock(m_mut);
    m_ticked = false;
    while (m_active) {
        m_cv.wait_for(lock, m_interval);
        m_ticked = true;
    }
}
