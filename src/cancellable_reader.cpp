#include "../include/cancellable_reader.hpp"

extern "C" {
#include <unistd.h>
}

CancellableReader::CancellableReader() {
    pipe(m_pipe);
}

void CancellableReader::setFile(FILE *file) {
    m_file = file;
    setFileDescriptor(fileno(file));
}

void CancellableReader::setFileDescriptor(int fd) {
    m_fd = fd;
    m_maxFd = (m_pipe[0] > m_fd ? m_pipe[0] : m_fd) + 1;
    FD_ZERO(&m_fd_set);
    /* FD_SET(m_fd, &m_fd_set); */
    /* FD_SET(m_pipe[0], &m_fd_set); */
}

ssize_t CancellableReader::getline(char **buf, size_t *size) {
    /* FD_SET(m_pipe[0], &m_fd_set); */
    /* FD_SET(m_fd, &m_fd_set); */
    /* if (select(m_maxFd, &m_fd_set, nullptr, nullptr, nullptr) <= 0) { */
    /*     return -1; */
    /* } */
    /* if (FD_ISSET(m_pipe[0], &m_fd_set)) { */
    /*     return -1; */
    /* } */
    return ::getline(buf, size, m_file);
}

ssize_t CancellableReader::read(char *buf, size_t n) {
    /* FD_SET(m_pipe[0], &m_fd_set); */
    /* FD_SET(m_fd, &m_fd_set); */
    /* if (select(m_maxFd, &m_fd_set, nullptr, nullptr, nullptr) <= 0) { */
    /*     return -1; */
    /* } */
    /* if (FD_ISSET(m_pipe[0], &m_fd_set)) { */
    /*     return -1; */
    /* } */
    return ::read(m_fd, buf, n);
}

void CancellableReader::cancel() {
    char byte;
    write(m_pipe[1], &byte, 1);
}

bool CancellableReader::blocked() {
    FD_SET(m_pipe[0], &m_fd_set);
    FD_SET(m_fd, &m_fd_set);
    timeval timeout {0, 0};
    if (select(m_maxFd, &m_fd_set, nullptr, nullptr, &timeout) <= 0) {

        return true;
    }
    return !(FD_ISSET(m_fd, &m_fd_set));
}
