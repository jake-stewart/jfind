#include "../include/cancellable_reader.hpp"

extern "C" {
#include <unistd.h>
}

CancellableReader::CancellableReader() {
    pipe(m_pipe);
}

void CancellableReader::setFile(FILE *file) {
    m_file = file;
    m_fileno = fileno(file);
    m_maxFd = (m_pipe[0] > m_fileno ? m_pipe[0] : m_fileno) + 1;
    FD_ZERO(&m_fd_set);
    FD_SET(m_fileno, &m_fd_set);
    FD_SET(m_pipe[0], &m_fd_set);
}

ssize_t CancellableReader::getline(char **buf, size_t *size) {
    select(m_maxFd, &m_fd_set, nullptr, nullptr, nullptr);

    if (FD_ISSET(m_pipe[0], &m_fd_set)) {
        return -1;
    }

    FD_SET(m_fileno, &m_fd_set);
    return ::getline(buf, size, m_file);
}

void CancellableReader::cancel() {
    char byte;
    write(m_pipe[1], &byte, 1);
}
