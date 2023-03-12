#ifndef MOUSE_EVENT_HPP
#define MOUSE_EVENT_HPP

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

#endif
