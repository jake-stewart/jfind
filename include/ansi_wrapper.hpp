#ifndef ANSI_WRAPPER_HPP
#define ANSI_WRAPPER_HPP

#include <csignal>
#include <cstdbool>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

extern "C" {
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
}

class AnsiWrapper
{
private:
    AnsiWrapper();

    bool m_inAlternateBuffer;
    bool m_mouseEnabled;
    FILE *m_outputFile;
    int m_inputFileNo;
    termios m_origTermios;

public:
    static AnsiWrapper &instance();

    void setInputFileNo(int fileNo);
    void setOutputFile(FILE *file);

    void move(unsigned int x, unsigned int y) const;
    void moveHome() const;

    void moveUp() const;
    void moveDown() const;
    void moveLeft() const;
    void moveRight() const;

    void moveUp(unsigned int amount) const;
    void moveDown(unsigned int amount) const;
    void moveLeft(unsigned int amount) const;
    void moveRight(unsigned int amount) const;

    void moveUpOrScroll() const;
    void moveDownOrScroll() const;

    void clearTerm() const;
    void clearTilEOL() const;
    void clearTilSOF() const;
    void clearTilEOF() const;

    void enableMouse();
    void disableMouse();

    void setAlternateBuffer(bool value);
    void setCursor(bool value) const;
    void setCtrlC(bool value);

    void initTerm();
    void initTermSimple();
    void restoreTerm();

    void saveCursor() const;
    void restoreCursor() const;

    void closeStdin();
};

#endif
