#ifndef CANCELLABLE_READER_HPP
#define CANCELLABLE_READER_HPP

#include <cstdio>
#include "logger.hpp"

extern "C" {
#include <poll.h>
}

class CancellableReader {
    FILE *m_file;
    int m_fd;
    pollfd m_fds[2];
    int m_pipe[2];
    bool m_cancelled = false;
    Logger m_logger = Logger("CancellableReader");

public:
    CancellableReader();
    void setFile(FILE *file);
    void setFileDescriptor(int fd);
    ssize_t getline(char **buf, size_t *size);
    ssize_t read(char *buf, size_t n);
    bool blocked();
    void cancel();
};

#endif
