#include "../include/logger.hpp"
#include <cstdarg>

bool Logger::c_enabled;
std::mutex Logger::c_mut;
FILE *Logger::c_fp;

bool Logger::open(const char *file) {
    if (c_enabled) {
        return false;
    }
    c_fp = fopen(file, "w+");
    c_enabled = !!c_fp;
    return c_enabled;
}

void Logger::close() {
    if (c_enabled) {
        c_enabled = false;
        fclose(c_fp);
    }
}

Logger::Logger(const char *name) {
    m_name = name;
}

void Logger::log(const char *fmt, ...) {
    if (!c_enabled) {
        return;
    }
    std::unique_lock lock(c_mut);
    fprintf(c_fp, "%s: ", m_name);
    va_list args;
    va_start(args, fmt);
    vfprintf(c_fp, fmt, args);
    va_end(args);
    fprintf(c_fp, "\n");
    fflush(c_fp);
}

