#ifndef SPINNER_HPP
#define SPINNER_HPP

#include "ansi_wrapper.hpp"
#include <chrono>
#include <cstdio>

class Spinner
{
    int m_x = 0;
    int m_y = 0;
    bool m_firstUpdate = false;
    bool m_firstUpdateComplete = false;
    int m_frame = 0;
    bool m_isSpinning = false;
    std::chrono::time_point<std::chrono::system_clock>
        m_lastFrameTime = std::chrono::system_clock::now();

    AnsiWrapper &ansi = AnsiWrapper::instance();

public:
    void setPosition(int x, int y);
    std::chrono::milliseconds frameTimeRemaining() const;
    void update();
    void draw() const;
    bool isSpinning() const;
    void setSpinning(bool value);
};

#endif
