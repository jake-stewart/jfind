#include "../include/cancellable_reader.hpp"

extern "C" {
#include <unistd.h>
}

void CancellableReader::setFile(FILE *file) {
    m_file = file;
    setFileDescriptor(fileno(file));
}

void CancellableReader::setFileDescriptor(int fd) {
    m_fd = fd;
}

ssize_t CancellableReader::getline(char **buf, size_t *size) {
    return ::getline(buf, size, m_file);
}

ssize_t CancellableReader::read(char *buf, size_t n) {
    return ::read(m_fd, buf, n);
}

void CancellableReader::cancel() {
}

bool CancellableReader::blocked() {
    fd_set fdset;
    FD_ZERO(&fdset);
    FD_SET(m_fd, &fdset);
    timeval timeout {0, 0};
    if (select(m_fd + 1, &fdset, nullptr, nullptr, &timeout) <= 0) {

        return true;
    }
    return !(FD_ISSET(m_fd, &fdset));
}
