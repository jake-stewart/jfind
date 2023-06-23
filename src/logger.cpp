#include "../include/logger.hpp"
#include <cstdarg>

bool Logger::open(const char *file) {
    m_startTime = std::chrono::system_clock::now();
    if (m_enabled) {
        return false;
    }
    m_fp = fopen(file, "w+");
    m_enabled = !!m_fp;
    if (m_enabled) {
        log("%5s %-16s %4s %s\n", "TIME", "LOCATION", "LINE", "MESSAGE");
    }
    return m_enabled;
}

void Logger::close() {
    if (m_enabled) {
        m_enabled = false;
        fclose(m_fp);
    }
}

unsigned long long Logger::timeSinceOpened() {
    std::chrono::system_clock::time_point now
        = std::chrono::system_clock::now();
    std::chrono::system_clock::time_point::duration duration
        = now - m_startTime;
    std::chrono::milliseconds ms
        = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
    return ms.count();
}

void Logger::log(const char *fmt, ...) {
    if (!m_enabled) {
        return;
    }
    va_list args;
    va_start(args, fmt);
    vfprintf(m_fp, fmt, args);
    va_end(args);
    fflush(m_fp);
}

Logger &Logger::instance() {
    static Logger singleton;
    return singleton;
}
