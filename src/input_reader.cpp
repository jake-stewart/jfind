#include <cstdio>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <sstream>
#include <map>

#include "../include/input_reader.hpp"

std::map<std::string, Key> createKeyLookup() {
    std::map<std::string, Key> lookup;
    lookup["[A"] = K_UP;
    lookup["[B"] = K_DOWN;
    lookup["[C"] = K_RIGHT;
    lookup["[D"] = K_LEFT;
    lookup["[1;2A"] = K_SHIFT_UP;
    lookup["[1;2B"] = K_SHIFT_DOWN;
    lookup["[1;2C"] = K_SHIFT_RIGHT;
    lookup["[1;2D"] = K_SHIFT_LEFT;
    lookup["[1;3A"] = K_ALT_UP;
    lookup["[1;3B"] = K_ALT_DOWN;
    lookup["[1;3C"] = K_ALT_RIGHT;
    lookup["[1;3D"] = K_ALT_LEFT;
    lookup["[1;5A"] = K_CTRL_UP;
    lookup["[1;5B"] = K_CTRL_DOWN;
    lookup["[1;5C"] = K_CTRL_RIGHT;
    lookup["[1;5D"] = K_CTRL_LEFT;
    lookup["[1;5I"] = K_CTRL_TAB;
    lookup["[1;6I"] = K_CTRL_SHIFT_TAB;
    lookup["\x1b[Z"] = K_ALT_SHIFT_TAB;
    lookup["OP"] = K_F1;
    lookup["OQ"] = K_F2;
    lookup["OR"] = K_F3;
    lookup["OS"] = K_F4;
    lookup["[15~"] = K_F5;
    lookup["[17~"] = K_F6;
    lookup["[18~"] = K_F7;
    lookup["[19~"] = K_F8;
    lookup["[20~"] = K_F9;
    lookup["[21~"] = K_F10;
    lookup["[3~"] = K_DELETE;

    return lookup;
}
const std::map<std::string, Key> ESC_KEY_LOOKUP = createKeyLookup();

int utf8CharLen(unsigned char ch) {
    if (ch < 128) {
        return 1;
    }
    else if (ch >> 5 == 0b110) {
        return 2;
    }
    else if (ch >> 4 == 0b1110) {
        return 3;
    }
    else if (ch >> 3 == 0b11110) {
        return 4;
    }
    return 0;
}

bool isContinuationByte(unsigned char ch) {
    return (ch >> 6) == 0b10;
}

int InputReader::getKey(Key *key) {
    if (m_mouseEvents.size()) {
        *key = K_MOUSE;
        return 0;
    }

    char ch = getch();
    switch (ch) {
        case K_ESCAPE:
            return parseEsc(key);
        case 1 ... (K_ESCAPE - 1):
        case (K_ESCAPE + 1) ... 127:
            *key = (Key)ch;
            return 0;
        default:
            parseUtf8(ch, key);
            return 1;
    }
}

bool InputReader::hasKey() {
    FD_ZERO(&m_set);
    FD_SET(m_fileDescriptor, &m_set);
    int sel = select(m_fileDescriptor + 1, &m_set, NULL, NULL, &m_timeout);
    return sel > 0;
}

InputReader::InputReader() {
    m_timeout.tv_sec = 0;
    m_timeout.tv_usec = 0;
    m_fileDescriptor = STDIN_FILENO;
}

void InputReader::setFileDescriptor(int fileDescriptor) {
    m_fileDescriptor = fileDescriptor;
}

char InputReader::getch() {
    char ch;
    if (read(m_fileDescriptor, &ch, 1) != 1) {
        fprintf(stderr, "read error or EOF\n");
    }
    return ch;
}

int InputReader::parseEsc(Key *key) {
    if (!hasKey()) {
        *key = K_ESCAPE;
        return 0;
    }

    std::stringstream ss;

    do {
        ss << getch();
    }
    while (hasKey());

    std::string seq = ss.str();
    if (seq.size() == 1) {
        return parseAltKey(seq[0], key);
    }
    else if (seq.size() > 2 && seq[0] == '[' && seq[1] == '<') {
        return parseMouse(seq, key);
    }
    else {
        return parseEsqSeq(seq, key);
    }
}

int InputReader::parseMouse(std::string& seq, Key *key) {
    int length;
    int idx = 0;

    m_mouseEvents.clear();

    while (idx < seq.size()) {
        MouseEvent event;
        int button;
        char pressed;
        int results = sscanf(seq.c_str() + idx, "[<%d;%d;%d%c%n", &button,
                &event.x, &event.y, &pressed, &length);
        idx += length + 1;

        if (results != 4) {
            continue;
        }

        switch (pressed) {
            case 'm':
                event.pressed = false;
                break;
            case 'M':
                event.pressed = true;
                break;
            default:
                continue;
        }

        event.dragged = false;
        switch (button) {
            case MB_LEFT:
                event.button = MB_LEFT;
                break;
            case MB_MIDDLE:
                event.button = MB_MIDDLE;
                break;
            case MB_RIGHT:
                event.button = MB_RIGHT;
                break;
            case MB_LEFT + 32:
                event.button = MB_LEFT;
                event.dragged = true;
                break;
            case MB_MIDDLE + 32:
                event.button = MB_MIDDLE;
                event.dragged = true;
                break;
            case MB_RIGHT + 32:
                event.button = MB_RIGHT;
                event.dragged = true;
                break;
            case MB_SCROLL_UP:
                event.button = MB_SCROLL_UP;
                break;
            case MB_SCROLL_DOWN:
                event.button = MB_SCROLL_DOWN;
                break;
            case MB_SCROLL_LEFT:
                event.button = MB_SCROLL_LEFT;
                break;
            case MB_SCROLL_RIGHT:
                event.button = MB_SCROLL_RIGHT;
                break;
            default:
                continue;
        }

        m_mouseEvents.push_back(event);
    }

    if (!m_mouseEvents.size()) {
        *key = K_UNKNOWN;
        return 1;
    }

    *key = K_MOUSE;
    return 0;
}

int InputReader::parseAltKey(char ch, Key *key) {
    switch (ch) {
        case 9:
            *key = K_ALT_TAB;
            return 0;
        case 13:
            *key = K_ALT_ENTER;
            return 0;
        case 32 ... 90:
        case 92 ... 127:
            *key = (Key)(K_ALT_SPACE + ch - 32);
            return 0;
        default:
            *key = K_UNKNOWN;
            return 1;
    }
}

int InputReader::parseEsqSeq(std::string& seq, Key *key) {
    std::map<std::string, Key>::const_iterator it;
    it = ESC_KEY_LOOKUP.find(seq);
    if (it == ESC_KEY_LOOKUP.end()) {
        *key = K_UNKNOWN;
        return 1;
    }
    *key = it->second;
    return 0;
}

int InputReader::parseUtf8(char ch, Key *key) {
    int length = utf8CharLen(ch);
    if (length < 2) {
        *key = K_UNKNOWN;
        return 1;
    }

    m_widechar[0] = ch;

    int i;
    for (i = 1; i < length; i++) {
        if (!hasKey()) {
            *key = K_UNKNOWN;
            return 1;
        }
        m_widechar[i] = getch();
        if (!isContinuationByte(m_widechar[i])) {
            *key = K_UNKNOWN;
            return 1;
        }
    }
    m_widechar[i] = 0;

    *key = K_UTF8;
    return 0;
}

std::string InputReader::getWideChar() {
    return std::string(m_widechar);
}

MouseEvent InputReader::getMouseEvent() {
    MouseEvent event = m_mouseEvents[0];
    m_mouseEvents.erase(m_mouseEvents.begin());
    return event;
}
