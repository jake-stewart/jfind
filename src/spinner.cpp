#include "../include/spinner.hpp"

using namespace std::chrono_literals;
using std::chrono::milliseconds;
using std::chrono::system_clock;

static const char *SPINNER[6] = {"⠇", "⠋", "⠙", "⠸", "⠴", "⠦"};
static const int SPINNER_SIZE = 6;

milliseconds Spinner::frameTimeRemaining() const {
    milliseconds remaining = 150ms -
        duration_cast<milliseconds>(system_clock::now() - m_lastFrameTime);
    if (remaining <= 0ms) {
        return 0ms;
    }
    return remaining;
}

void Spinner::setPosition(int x, int y) {
    m_x = x;
    m_y = y;
}

void Spinner::update() {
    if (m_firstUpdate) {
        m_firstUpdate = false;
        return;
    }
    m_firstUpdateComplete = true;
    draw();
    m_frame = (m_frame + 1) % SPINNER_SIZE;
    m_lastFrameTime = system_clock::now();
}

void Spinner::draw() const {
    if (m_firstUpdateComplete) {
        ansi.move(m_x, m_y);
        fprintf(stderr, "%s", SPINNER[m_frame]);
    }
}

bool Spinner::isSpinning() const {
    return m_isSpinning;
}

void Spinner::setSpinning(bool value) {
    if (!m_isSpinning) {
        m_lastFrameTime = system_clock::now();
        m_firstUpdate = true;
        m_firstUpdateComplete = false;
    }
    m_isSpinning = value;
}
