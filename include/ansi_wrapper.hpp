#ifndef ANSI_WRAPPER_HPP
#define ANSI_WRAPPER_HPP

#include <unistd.h>
#include <stdint.h>
#include <stddef.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <stdbool.h>


class AnsiWrapper {
private:
    AnsiWrapper();

    bool m_inAlternateBuffer;
    bool m_mouseEnabled;
    FILE *m_outputFile;
    int m_inputFileNo;
    termios m_origTermios;

public:
    static AnsiWrapper& instance();

    void setInputFileNo(int fileNo);
    void setOutputFile(FILE *file);

    void move(unsigned int x, unsigned int y);
    void moveHome();

    void moveUp();
    void moveDown();
    void moveLeft();
    void moveRight();

    void moveUp(unsigned int amount);
    void moveDown(unsigned int amount);
    void moveLeft(unsigned int amount);
    void moveRight(unsigned int amount);

    void moveUpOrScroll();
    void moveDownOrScroll();

    void clearTerm();
    void clearTilEOL();
    void clearTilSOF();
    void clearTilEOF();

    void enableMouse();
    void disableMouse();

    void setAlternateBuffer(bool value);
    void setCursor(bool value);
    void setCtrlC(bool value);

    void initTerm();
    void initTermSimple();
    void restoreTerm();

    void saveCursor();
    void restoreCursor();

    void closeStdin();
};

#endif
