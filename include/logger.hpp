#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <cstdio>
#include <mutex>

class Logger {
    bool m_enabled;
    std::mutex m_mut;
    FILE *m_fp;

public:
    static Logger& instance();
    bool open(const char *file);
    void log(const char *fmt, ...);

    ~Logger();
};

#endif
