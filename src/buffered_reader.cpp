#include "../include/buffered_reader.hpp"

#include <cstdio>
#include <cstdlib>

extern "C" {
#include <unistd.h>
#include <string.h>
}

#include "../include/logger.hpp"

#define START_CAPACITY 4096

BufferedReader::BufferedReader() {
    reset();
}

const char* BufferedReader::getlines(int numLines) {
    while (true) {
        while (m_index < m_filled) {
            if (m_buffer[m_index] == '\n') {
                m_buffer[m_index++] = 0;
                if (!--numLines) {
                    int prevLineStart = m_lineStart;
                    m_lineStart = m_index;
                    return m_buffer + prevLineStart;
                }
            }
            m_index++;
        }
        if (m_filled >= m_capacity) {
            size_t newCapacity = m_capacity * 1.5;
            char *newBuffer = (char*)malloc(newCapacity);
            if (!newBuffer) {
                LOG("failed to malloc additional buffer, errno=%d", errno);
                return nullptr;
            }
            m_buffers.push_back(newBuffer);
            m_capacity = newCapacity;
            m_filled = m_filled - m_lineStart;
            memcpy(newBuffer, m_buffer + m_lineStart, m_filled);
            m_index = m_filled;
            m_lineStart = 0;
            m_buffer = newBuffer;
        }
        ssize_t bytesRead = read(m_fd,
                m_buffer + m_filled, m_capacity - m_filled);
        if (bytesRead <= 0) {
            LOG("read failed, fd=%d, errno=%d filled=%d, capacity=%d", m_fd, errno, m_filled, m_capacity);
            return nullptr;
        }
        m_filled += bytesRead;
    }
}

bool BufferedReader::reset() {
    m_filled = 0;
    m_index = 0;
    m_lineStart = 0;
    m_capacity = START_CAPACITY;
    m_buffer = (char*)malloc(m_capacity);
    m_buffers.clear();
    if (!m_buffer) {
        LOG("failed to reset and malloc new buffer, errno=%d", errno);
        return false;
    }
    m_buffers.push_back(m_buffer);
    return true;
}

std::vector<char *> BufferedReader::getBuffers() {
    return m_buffers;
}

void BufferedReader::setFd(int fd) {
    LOG("setting fd to %d", fd);
    m_fd = fd;
}
