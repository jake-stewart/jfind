#include "../include/logger.hpp"
#include <stdarg.h>

bool Logger::open(const char *file) {
    if (m_enabled) {
        return false;
    }
    m_fp = fopen(file, "w+");
    m_enabled = !!m_fp;
    return m_enabled;
}

void Logger::log(const char *fmt, ...) {
    if (!m_enabled) {
        return;
    }
    std::unique_lock lock(m_mut);
    va_list args;
    va_start(args, fmt);
    vfprintf(m_fp, fmt, args);
    va_end(args);
    fprintf(m_fp, "\n");
    fflush(m_fp);
}

Logger::~Logger() {
    if (m_enabled) {
        m_enabled = false;
        fclose(m_fp);
    }
}

Logger& Logger::instance() {
    static Logger singleton;
    return singleton;
}
