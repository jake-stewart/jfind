#include "../include/cancellable_reader.hpp"

extern "C" {
#include <unistd.h>
}

#define READ 0
#define WRITE 1

#define USER_FD 0
#define CANCEL_FD 1

CancellableReader::CancellableReader() {
    pipe(m_pipe);
}

void CancellableReader::setFile(FILE *file) {
    m_file = file;
    setFileDescriptor(fileno(file));
}

void CancellableReader::setFileDescriptor(int fd) {
    m_fd = fd;
    m_fds[USER_FD] = {m_fd, POLLIN, 0};
    m_fds[CANCEL_FD] = {m_pipe[READ], POLLIN, 0};
}

ssize_t CancellableReader::getline(char **buf, size_t *size) {
    if (m_cancelled) {
        return -1;
    }
    int ret = poll(m_fds, 2, -1);
    if (ret == -1 || m_fds[CANCEL_FD].revents & POLLIN) {
        return -1;
    }
    return ::getline(buf, size, m_file);
}

ssize_t CancellableReader::read(char *buf, size_t n) {
    if (m_cancelled) {
        return -1;
    }
    m_logger.log("blocked reading...");
    int ret = poll(m_fds, 2, -1);
    m_logger.log("done %x", ret);
    if (ret == -1 || m_fds[CANCEL_FD].revents & POLLIN) {
        m_logger.log("error or cancel");
        return -1;
    }
    if (m_fds[USER_FD].revents & POLLIN) {
        m_logger.log("user has data APPARENTLY");
        return ::read(m_fd, buf, n);
    }
    else {
        return ::read(m_fd, buf, n);
        m_logger.log("%d", m_fds[USER_FD].revents);
        m_logger.log("no data? no bitches? poll() bug?");
        return -1;
    }
}

bool CancellableReader::blocked() {
    if (m_cancelled) {
        return true;
    }
    int ret = poll(m_fds, 2, -1);
    if (ret == -1 || m_fds[CANCEL_FD].revents & POLLIN) {
        return true;
    }
    return !(m_fds[USER_FD].revents & POLLIN);
}

void CancellableReader::cancel() {
    m_logger.log("Cancel");
    m_cancelled = true;
    char byte;
    write(m_pipe[WRITE], &byte, 1);
}
