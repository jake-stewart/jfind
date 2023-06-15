#include "../include/interval_thread.hpp"

void IntervalThread::setInterval(std::chrono::milliseconds interval) {
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

std::chrono::milliseconds IntervalThread::getRemaining() {
    auto now = std::chrono::system_clock::now();
    auto duration = now - m_start;
    auto elapsedMilliseconds = std::chrono::duration_cast<
        std::chrono::milliseconds>(duration);
    return m_interval - elapsedMilliseconds;
}

void IntervalThread::threadFunc() {
    std::unique_lock lock(m_mut);
    m_ticked = false;
    while (m_active) {
        m_start = std::chrono::system_clock::now();
        m_cv.wait_for(lock, m_interval);
        m_ticked = true;
    }
}
