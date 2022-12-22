#ifndef INPUT_READER_HPP
#define INPUT_READER_HPP

#include "key.hpp"
#include <string>
#include <vector>
#include <sys/select.h>

bool isContinuationByte(unsigned char ch);
int utf8CharLen(unsigned char ch);

enum MouseButton {
    MB_NONE = -1,
    MB_LEFT = 0,
    MB_MIDDLE = 1,
    MB_RIGHT = 2,
    MB_SCROLL_UP = 64,
    MB_SCROLL_DOWN = 65,
    MB_SCROLL_RIGHT = 66,
    MB_SCROLL_LEFT = 67,
};

struct MouseEvent {
    MouseButton button;
    bool dragged;
    bool pressed;
    int x;
    int y;
};

class InputReader {
    public:
        int getKey(Key *key);
        bool hasKey();
        std::string getWideChar();
        MouseEvent getMouseEvent();
        InputReader();

    private:
        fd_set m_set;
        struct timeval m_timeout;
        char m_widechar[4];
        std::vector<MouseEvent> m_mouse_events;
        MouseEvent m_mouse_event;

        char getch();
        int parseEsc(Key *key);
        int parseAltKey(char ch, Key *key);
        int parseMouse(std::string& seq, Key *key);
        int parseEsqSeq(std::string& seq, Key *key);
        int parseUtf8(char ch, Key *key);

};

#endif
