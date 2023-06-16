#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <cstdio>
#include <mutex>

class Logger
{
    static bool c_enabled;
    static std::mutex c_mut;
    static FILE *c_fp;

    const char *m_name;

public:
    static bool open(const char *file);
    static void close();

    Logger(const char *name);
    void log(const char *fmt, ...);
    void logUnsafe(const char *fmt, ...);
};

#endif
