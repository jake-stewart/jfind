#ifndef DOUBLE_BUFFER_HPP
#define DOUBLE_BUFFER_HPP

template <class T>
class DoubleBuffer
{
public:
    void swap() {
        m_isSwapped = !m_isSwapped;
    }

    T &getPrimary() {
        return m_isSwapped ? m_first : m_second;
    }

    T &getSecondary() {
        return m_isSwapped ? m_second : m_first;
    }

private:
    bool m_isSwapped = false;
    T m_first;
    T m_second;
};

#endif
