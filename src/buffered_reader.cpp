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

const char* BufferedReader::getline() {
    int result = m_offset;
    while (true) {
        while (m_offset < m_size) {
            if (m_buffer[m_offset] == '\n') {
                m_buffer[m_offset++] = 0;
                return m_buffer + result;
            }
            m_offset++;
        }
        if (m_size >= m_capacity) {
            m_capacity *= 1.5;
            char *newBuffer = (char*)malloc(m_capacity);
            if (!newBuffer) {
                return nullptr;
            }
            m_buffers.push_back(newBuffer);
            m_size = m_size - result;
            m_offset = 0;
            result = 0;
            memcpy(newBuffer, m_buffer + result, m_size);
            m_buffer = newBuffer;
        }
        ssize_t bytesRead = read(0, m_buffer + m_size, m_capacity - m_size);
        if (bytesRead <= 0) {
            return nullptr;
        }
        m_size += bytesRead;
    }
}

bool BufferedReader::reset() {
    m_size = 0;
    m_offset = 0;
    m_capacity = START_CAPACITY;
    m_buffer = (char*)malloc(m_capacity);
    m_buffers.clear();
    if (!m_buffer) {
        return false;
    }
    m_buffers.push_back(m_buffer);
    return true;
}

std::vector<char *> BufferedReader::getBuffers() {
    return m_buffers;
}
