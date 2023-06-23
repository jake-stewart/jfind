#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <cstdio>
#include <mutex>

#include "util.hpp"

class Logger {
    bool m_enabled;
    FILE *m_fp;
    std::chrono::system_clock::time_point m_startTime;

public:
    static Logger& instance();
    unsigned long long timeSinceOpened();
    bool open(const char *file);
    void close();
    void log(const char *fmt, ...);
};

#define LOG(fmt, ...) \
    Logger::instance().log( \
        "%5lld %-16s %4d " fmt "\n", \
        Logger::instance().timeSinceOpened(), \
        fileStem(fileName(__FILE__)).c_str(), \
        __LINE__, \
        ##__VA_ARGS__ \
    )

#endif
