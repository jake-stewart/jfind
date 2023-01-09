#include "../include/ansi_wrapper.hpp"

#include <sys/stat.h>
#include <fcntl.h>

#define ANSI_ESC "\x1b["

bool inAlternateBuffer = false;
bool ignoreSigint = false;
bool mouseEnabled = false;
FILE *outputFile = stdout;
int inputFileNo = STDIN_FILENO;
termios origTermios;

static void (*resizeCallback)(int, int) = nullptr;

void move(unsigned int x, unsigned int y) {
    fprintf(outputFile, ANSI_ESC "%u;%uH", y + 1, x + 1);
}

void moveHome() {
    fprintf(outputFile, ANSI_ESC "H");
}

void moveUp() {
    fprintf(outputFile, ANSI_ESC "A");
}

void moveDown() {
    fprintf(outputFile, ANSI_ESC "B");
}

void moveRight() {
    fprintf(outputFile, ANSI_ESC "C");
}

void moveLeft() {
    fprintf(outputFile, ANSI_ESC "D");
}

void moveUp(unsigned int amount) {
    fprintf(outputFile, ANSI_ESC "%uA", amount);
}

void moveDown(unsigned int amount) {
    fprintf(outputFile, ANSI_ESC "%uB", amount);
}

void moveRight(unsigned int amount) {
    fprintf(outputFile, ANSI_ESC "%uC", amount);
}

void moveLeft(unsigned int amount) {
    fprintf(outputFile, ANSI_ESC "%uD", amount);
}

void moveUpOrScroll() {
    fprintf(outputFile, "\x1bM");
}

void moveDownOrScroll() {
    fprintf(outputFile, "\n");
}

void enableMouse() {
    if (!mouseEnabled) {
        mouseEnabled = true;
        fprintf(outputFile, ANSI_ESC "?1002h" ANSI_ESC "?1015h" ANSI_ESC "?1006h");
    }
}

void disableMouse() {
    if (mouseEnabled) {
        mouseEnabled = false;
        fprintf(outputFile, ANSI_ESC "?1000l");
    }
}

void setAlternateBuffer(bool value) {
    if (inAlternateBuffer == value) {
        return;
    }
    inAlternateBuffer = value;
    fprintf(outputFile, ANSI_ESC "?1049%c", value ? 'h' : 'l');
}

void clearTilEOL() {
    fprintf(outputFile, ANSI_ESC "K");
}

void clearTilSOF() {
    fprintf(outputFile, ANSI_ESC "1J");
}

void clearTilEOF() {
    fprintf(outputFile, ANSI_ESC "2J");
}

void clearTerm() {
    fprintf(outputFile, ANSI_ESC "2J");
}

void setCursor(bool value) {
    fprintf(outputFile, ANSI_ESC "?25%c", value ? 'h' : 'l');
}

void exitGracefully(int sig) {
    restoreTerm();
    exit(1);
}

void setCtrlC(bool value) {
    ignoreSigint = value;
}

void onSigint(int sig) {
    if (!ignoreSigint) {
        exitGracefully(sig);
    }
}

void setInputFileNo(int fileNo) {
    inputFileNo = fileNo;
}

void setOutputFile(FILE *file) {
    outputFile = file;
}

void onSigResize(int sig) {
    winsize ws;
    if (ioctl(fileno(outputFile), TIOCGWINSZ, &ws)) {
        restoreTerm();
        fprintf(outputFile, "Failed to query terminal size\n");
        exit(1);
    }
    if (resizeCallback != nullptr) {
        resizeCallback(ws.ws_col, ws.ws_row);
    }
}

void saveCursor() {
    fprintf(outputFile, "\x1b" "7");
}

void restoreCursor() {
    fprintf(outputFile, "\x1b" "8");
}

void restoreTerm(void) {
    setAlternateBuffer(true);
    disableMouse();
    fprintf(outputFile, "\x1b[0m");
    clearTerm();
    setCursor(true);
    setAlternateBuffer(false);
    tcsetattr(inputFileNo, TCSANOW, &origTermios);

    signal(SIGWINCH, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGINT, SIG_DFL);
}

// char stdoutBuffer[50000];

void initTerm(void) {
    // setvbuf(stdout, NULL, _IONBF, 0);
    // setvbuf(stdout, stdoutBuffer, _IOFBF, sizeof(stdoutBuffer));

    tcgetattr(inputFileNo, &origTermios);
    termios raw = origTermios;

    raw.c_iflag &= ~(ICRNL);  // differentiate newline and linefeed
    raw.c_lflag &= ISIG;  // generate exit signals
    raw.c_lflag &= ~(ECHO | ICANON);  // disable echo and cannonical mode

    tcsetattr(inputFileNo, TCSANOW, &raw);

    signal(SIGWINCH, onSigResize);
    signal(SIGTERM, exitGracefully);
    signal(SIGQUIT, exitGracefully);
    signal(SIGINT, onSigint);

    setAlternateBuffer(true);
    clearTerm();
    moveHome();
    onSigResize(0);
}

void registerResizeCallback(void (*func)(int, int)) {
    resizeCallback = func;
}

void closeStdin() {
    int fd = open("/dev/null", O_RDONLY);
    dup2(fd, 0);
    close(fd);
}
