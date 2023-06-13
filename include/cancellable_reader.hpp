#ifndef CANCELLABLE_READER_HPP
#define CANCELLABLE_READER_HPP

#include <cstdio>

extern "C" {
#include <sys/select.h>
}

class CancellableReader {
    FILE *m_file;
    int m_fileno;

    int m_pipe[2];
    fd_set m_fd_set;
    int m_maxFd;

public:
    CancellableReader();
    void setFile(FILE *file);
    ssize_t getline(char **buf, size_t *size);
    void cancel();
};

#endif
