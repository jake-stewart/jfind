#ifndef CANCELLABLE_READER_HPP
#define CANCELLABLE_READER_HPP

#include <cstdio>
#include "logger.hpp"

extern "C" {
#include <sys/select.h>
#include <poll.h>
}

class CancellableReader {
    FILE *m_file;
    int m_fd;
    Logger m_logger = Logger("CancellableReader");

public:
    void setFile(FILE *file);
    void setFileDescriptor(int fd);
    ssize_t getline(char **buf, size_t *size);
    ssize_t read(char *buf, size_t n);
    bool blocked();
    void cancel();
};

#endif
