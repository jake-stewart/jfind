#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <map>
#include <sstream>

extern "C" {
#include <sys/select.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>
}

#include "../include/input_reader.hpp"

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::system_clock;

static std::map<std::string, Key> createKeyLookup() {
    std::map<std::string, Key> lookup;
    lookup["A"] = K_UP;
    lookup["B"] = K_DOWN;
    lookup["C"] = K_RIGHT;
    lookup["D"] = K_LEFT;
    lookup["[A"] = K_UP;
    lookup["[B"] = K_DOWN;
    lookup["[C"] = K_RIGHT;
    lookup["[D"] = K_LEFT;
    lookup["OA"] = K_UP;
    lookup["OB"] = K_DOWN;
    lookup["OC"] = K_RIGHT;
    lookup["OD"] = K_LEFT;
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
static const std::map<std::string, Key> ESC_KEY_LOOKUP = createKeyLookup();

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

bool InputReader::getKey(Key *key) {
    char c;
    if (!getch(&c)) {
        return false;
    }
    switch (c) {
        case -1:
            *key = K_ERROR;
            return true;
        case K_ESCAPE:
            return parseEsc(key);
        case 1 ...(K_ESCAPE - 1):
        case (K_ESCAPE + 1)... 127:
            *key = (Key)c;
            return true;
        default:
            return parseUtf8(c, key);
    }
}

void InputReader::setFileDescriptor(int fd) {
    m_reader.setFileDescriptor(fd);
}

bool InputReader::getch(char *c) {
    if (m_reader.read(c, 1) < 0) {
        return false;
    }
    return true;
}

bool InputReader::parseEsc(Key *key) {
    if (m_reader.blocked()) {
        *key = K_ESCAPE;
        return true;
    }

    std::stringstream ss;

    do {
        char c;
        if (!getch(&c)) {
            return false;
        }
        ss << c;
    } while (!m_reader.blocked());

    std::string seq = ss.str();
    if (seq.size() == 1) {
        parseAltKey(seq[0], key);
    }
    else if (seq.size() > 2 && seq[0] == '[' && seq[1] == '<') {
        parseMouse(seq, key);
    }
    else {
        parseEscSeq(seq, key);
    }
    return true;
}

void InputReader::parseMouse(std::string &seq, Key *key) {
    int length;
    int idx = 0;

    m_mouseEvents.clear();

    while (idx < seq.size()) {
        MouseEvent event;
        int button;
        char pressed;
        int results = sscanf(
            seq.c_str() + idx, "[<%d;%d;%d%c%n", &button, &event.x, &event.y,
            &pressed, &length
        );
        idx += length + 1;

        if (results != 4) {
            continue;
        }

        switch (pressed) {
            case 'm':
                event.pressed = false;
                event.numClicks = 0;
                break;
            case 'M':
                event.pressed = true;
                event.numClicks = 1;
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

        if (event.pressed == false && event.button != m_lastClickButton ||
            event.dragged) {
            m_clickCount = 0;
            m_lastClickButton = MB_NONE;
        }
        else if (event.pressed) {
            if (event.button == m_lastClickButton) {
                milliseconds delta = duration_cast<milliseconds>(
                    system_clock::now() - m_lastClickTime
                );
                if (delta.count() < 250) {
                    m_clickCount++;
                    event.numClicks += m_clickCount;
                }
                else {
                    m_clickCount = 0;
                }
            }
            else {
                m_clickCount = 0;
            }
            m_lastClickButton = event.button;
            m_lastClickTime = system_clock::now();
        }

        m_mouseEvents.push_back(event);
    }

    if (!m_mouseEvents.size()) {
        *key = K_UNKNOWN;
    }

    *key = K_MOUSE;
}

void InputReader::parseAltKey(char c, Key *key) {
    switch (c) {
        case 9:
            *key = K_ALT_TAB;
            break;
        case 13:
            *key = K_ALT_ENTER;
            break;
        case 32 ... 90:
        case 92 ... 127:
            *key = (Key)(K_ALT_SPACE + c - 32);
            break;
        default:
            *key = K_UNKNOWN;
            break;
    }
}

void InputReader::parseEscSeq(std::string &seq, Key *key) {
    std::map<std::string, Key>::const_iterator it;
    it = ESC_KEY_LOOKUP.find(seq);
    if (it == ESC_KEY_LOOKUP.end()) {
        *key = K_UNKNOWN;
    }
    else {
        *key = it->second;
    }
}

bool InputReader::parseUtf8(char ch, Key *key) {
    int length = utf8CharLen(ch);
    if (length < 2) {
        *key = K_UNKNOWN;
        return true;
    }

    m_widechar[0] = ch;

    int i;
    for (i = 1; i < length; i++) {
        if (m_reader.blocked()) { // todo test
            *key = K_UNKNOWN;
            return true;
        }
        char c;
        if (!getch(&c)) {
            return false;
        }
        m_widechar[i] = c;
        if (!isContinuationByte(m_widechar[i])) {
            *key = K_UNKNOWN;
            return true;
        }
    }
    m_widechar[i] = 0;

    *key = K_UTF8;
    return true;
}

void InputReader::onStart() {
    m_logger.log("started");
}

void InputReader::onLoop() {
    Key key;
    if (!getKey(&key)) {
        end();
        return;
    }
    switch (key) {
        case K_MOUSE:
            m_dispatch.dispatch(std::make_shared<KeyEvent>(key, m_mouseEvents));
            break;
        case K_UTF8:
            m_dispatch.dispatch(std::make_shared<KeyEvent>(key, m_widechar));
            break;
        default:
            m_dispatch.dispatch(std::make_shared<KeyEvent>(key));
            break;
    }
}

void InputReader::onEvent(std::shared_ptr<Event> event) {
    m_logger.log("received %s", getEventNames()[event->getType()]);
}
