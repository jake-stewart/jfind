#include "../include/ansi_wrapper.hpp"

extern "C" {
#include <fcntl.h>
#include <sys/stat.h>
}

#define ANSI_ESC "\x1b["

// output buffer so that we can flush when done drawing
// without output buffer, cursor may jump around causing slight flicker
static char outputBuffer[50000];

AnsiWrapper::AnsiWrapper() {
    m_inAlternateBuffer = false;
    m_mouseEnabled = false;
    m_outputFile = stdout;
    m_inputFileNo = STDIN_FILENO;
};

AnsiWrapper &AnsiWrapper::instance() {
    static AnsiWrapper singleton;
    return singleton;
}

void AnsiWrapper::move(unsigned int x, unsigned int y) const {
    fprintf(m_outputFile, ANSI_ESC "%u;%uH", y + 1, x + 1);
}

void AnsiWrapper::moveHome() const {
    fprintf(m_outputFile, ANSI_ESC "H");
}

void AnsiWrapper::moveUp() const {
    fprintf(m_outputFile, ANSI_ESC "A");
}

void AnsiWrapper::moveDown() const {
    fprintf(m_outputFile, ANSI_ESC "B");
}

void AnsiWrapper::moveRight() const {
    fprintf(m_outputFile, ANSI_ESC "C");
}

void AnsiWrapper::moveLeft() const {
    fprintf(m_outputFile, ANSI_ESC "D");
}

void AnsiWrapper::moveUp(unsigned int amount) const {
    fprintf(m_outputFile, ANSI_ESC "%uA", amount);
}

void AnsiWrapper::moveDown(unsigned int amount) const {
    fprintf(m_outputFile, ANSI_ESC "%uB", amount);
}

void AnsiWrapper::moveRight(unsigned int amount) const {
    fprintf(m_outputFile, ANSI_ESC "%uC", amount);
}

void AnsiWrapper::moveLeft(unsigned int amount) const {
    fprintf(m_outputFile, ANSI_ESC "%uD", amount);
}

void AnsiWrapper::scrollUp() const {
    fprintf(m_outputFile, ANSI_ESC "S");
}

void AnsiWrapper::scrollDown() const {
    fprintf(m_outputFile, ANSI_ESC "T");
}

void AnsiWrapper::scrollLeft() const {
    fprintf(m_outputFile, ANSI_ESC "@");
}

void AnsiWrapper::scrollRight() const {
    fprintf(m_outputFile, ANSI_ESC "A");
}

void AnsiWrapper::enableMouse() {
    if (!m_mouseEnabled) {
        m_mouseEnabled = true;
        fprintf(
            m_outputFile, ANSI_ESC "?1002h" ANSI_ESC "?1015h" ANSI_ESC "?1006h"
        );
    }
}

void AnsiWrapper::disableMouse() {
    if (m_mouseEnabled) {
        m_mouseEnabled = false;
        fprintf(
            m_outputFile, ANSI_ESC "?1002l" ANSI_ESC "?1015l" ANSI_ESC "?1006l"
        );
    }
}

void AnsiWrapper::setAlternateBuffer(bool value) {
    if (m_inAlternateBuffer == value) {
        return;
    }
    m_inAlternateBuffer = value;
    fprintf(m_outputFile, ANSI_ESC "?1049%c", value ? 'h' : 'l');
}

void AnsiWrapper::clearTilEOL() const {
    fprintf(m_outputFile, ANSI_ESC "K");
}

void AnsiWrapper::clearTilSOF() const {
    fprintf(m_outputFile, ANSI_ESC "1J");
}

void AnsiWrapper::clearTilEOF() const {
    fprintf(m_outputFile, ANSI_ESC "2J");
}

void AnsiWrapper::clearTerm() const {
    fprintf(m_outputFile, ANSI_ESC "2J");
}

void AnsiWrapper::setCursor(bool value) const {
    fprintf(m_outputFile, ANSI_ESC "?25%c", value ? 'h' : 'l');
}

void AnsiWrapper::setInputFileNo(int fileNo) {
    m_inputFileNo = fileNo;
}

void AnsiWrapper::setOutputFile(FILE *file) {
    m_outputFile = file;
}

void AnsiWrapper::saveCursor() const {
    fprintf(
        m_outputFile,
        "\x1b"
        "7"
    );
}

void AnsiWrapper::restoreCursor() const {
    fprintf(
        m_outputFile,
        "\x1b"
        "8"
    );
}

void AnsiWrapper::restoreTerm(void) {
    setAlternateBuffer(true);
    disableMouse();
    fprintf(m_outputFile, "\x1b[0m");
    clearTerm();
    setCursor(true);
    resetScrollRegion();
    setAlternateBuffer(false);
    fflush(m_outputFile);
    tcsetattr(m_inputFileNo, TCSANOW, &m_origTermios);
    signal(SIGWINCH, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
}

void AnsiWrapper::initTerm(void) {
    setvbuf(m_outputFile, outputBuffer, _IOFBF, sizeof(outputBuffer));
    // setvbuf(m_outputFile, nullptr, _IOFBF, 0);

    tcgetattr(m_inputFileNo, &m_origTermios);
    termios term = m_origTermios;

    term.c_lflag &= ECHOCTL; // stops newline when <c-c> before eof
    term.c_iflag &= ~ICRNL;  // differentiate newline and linefeed
    term.c_iflag &= ~IXON;   // allow ctrl+s ctrl+q keys
    term.c_lflag &= ISIG;    // generate exit signals
    term.c_lflag &= ~ECHO;   // disable echoing keys back to user
    term.c_lflag &= ~ICANON; // disable cannonical mode

    tcsetattr(m_inputFileNo, TCSANOW, &term);

    setAlternateBuffer(true);
    clearTerm();
    moveHome();
}

void AnsiWrapper::setScrollRegion(unsigned int minRow, unsigned int maxRow) {
    m_scrollRegionEnabled = true;
    fprintf(m_outputFile, ANSI_ESC "%d;%dr", minRow + 1, maxRow + 1);
}

void AnsiWrapper::resetScrollRegion() {
    if (!m_scrollRegionEnabled) {
        return;
    }
    m_scrollRegionEnabled = false;
    fprintf(m_outputFile, ANSI_ESC "r");
}
