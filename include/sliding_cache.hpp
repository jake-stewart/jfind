#ifndef SLIDING_CACHE_HPP
#define SLIDING_CACHE_HPP

#include "logger.hpp"
#include "util.hpp"
#include <functional>

template <class T>
class SlidingCache
{
public:
    SlidingCache() {
        m_reserve = 128;
        m_cache = new T[m_reserve];
        m_size = 0;
        m_idx = 0;
        m_offset = 0;
    }

    ~SlidingCache() {
        delete[] m_cache;
    }

    void setDatasource(std::function<int(T *buffer, int idx, int n)> datasource
    ) {
        m_datasource = datasource;
    }

    T *get(int i) {
        if (i >= m_offset + m_reserve) {
            if (i - m_offset >= m_reserve + m_reserve / 2) {
                refresh(i);
            }
            else {
                int n = loadData(m_offset + m_reserve, m_reserve / 2);
                if (n > 0) {
                    m_size = m_offset + m_reserve + n;
                    m_offset += m_reserve / 2;
                    m_idx = m_idx ? 0 : m_reserve / 2;
                }
            }
        }

        else if (i < m_offset) {
            if (i - m_offset < m_reserve / -2) {
                refresh(i);
            }
            else {
                m_idx = m_idx ? 0 : m_idx + m_reserve / 2;
                m_offset -= m_reserve / 2;
                loadData(m_offset, m_reserve / 2);
            }
        }

        if (i >= m_size) {
            return nullptr;
        }

        int idx = mod(i + m_idx - m_offset, m_reserve);

        T *x = &m_cache[idx];
        return x;
    }

    void refresh() {
        m_size = 0;
        refresh(0);
    }

    int getReserve() const {
        return m_reserve;
    }

    void setReserve(int n) {
        m_reserve = n;
        delete m_cache;
        m_cache = new T[m_reserve];
        m_idx = 0;
        refresh();
    }

    int size() const {
        return m_size;
    }

private:
    T *m_cache = nullptr;
    int m_reserve;
    int m_size;
    int m_offset;
    int m_idx;
    std::function<int(T *buffer, int idx, int n)> m_datasource;

    void refresh(int offset) {
        int roundedOffset = offset - mod(offset, m_reserve);
        int n = m_datasource(m_cache, roundedOffset, m_reserve);
        if (n > 0) {
            m_idx = 0;
            m_offset = roundedOffset;
            m_size = m_offset + n;
        }
    }

    int loadData(int offset, int amount) {
        return m_datasource(m_cache + m_idx, offset, amount);
    }
};

#endif
