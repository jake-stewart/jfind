#include <cstdio>
#include <cstring>
#include <cerrno>
#include <sstream>
#include <map>
#include <iostream>
#include <csignal>

extern "C" {
#include <termios.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/select.h>
}

#include "../include/input_reader.hpp"

using std::chrono::system_clock;
using std::chrono::milliseconds;
using std::chrono::duration_cast;

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
    char ch = getch();
    switch (ch) {
        case -1:
            *key = K_ERROR;
            return false;
        case K_ESCAPE:
            return parseEsc(key);
        case 1 ... (K_ESCAPE - 1):
        case (K_ESCAPE + 1) ... 127:
            *key = (Key)ch;
            return true;
        default:
            return parseUtf8(ch, key);
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

    pipe(m_pipe);
    m_dispatch.subscribe(this, QUIT_EVENT);
}

void InputReader::setFileDescriptor(int fileDescriptor) {
    m_fileDescriptor = fileDescriptor;
}

char InputReader::getch() {
    fd_set read_fds, except_fds;
    FD_ZERO(&read_fds);
    FD_SET(m_fileDescriptor, &read_fds);
    FD_SET(m_pipe[0], &read_fds);

    FD_ZERO(&except_fds);
    FD_SET(m_fileDescriptor, &except_fds);
    FD_SET(m_pipe[0], &except_fds);

    int maxfd = m_fileDescriptor > m_pipe[0] ? m_fileDescriptor : m_pipe[0];
    int activity = select(maxfd + 1, &read_fds, nullptr, &except_fds, nullptr);

    switch (activity) {
        case -1:
        case 0:
            m_logger.log("select error");
            m_dispatch.dispatch(std::make_shared<QuitEvent>());
            break;
    }

    if (FD_ISSET(m_fileDescriptor, &read_fds)) {
        char ch;
        if (read(m_fileDescriptor, &ch, 1) != 1) {
            return -1;
        }
        return ch;
    }

    return -1;
}

int InputReader::parseEsc(Key *key) {
    if (!hasKey()) {
        *key = K_ESCAPE;
        return true;
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

        if (event.pressed == false && event.button != m_lastClickButton || event.dragged) {
            m_clickCount = 0;
            m_lastClickButton = MB_NONE;
        }
        else if (event.pressed) {
            if (event.button == m_lastClickButton) {
                milliseconds delta = duration_cast<milliseconds>(
                        system_clock::now() - m_lastClickTime);
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
        return false;
    }

    *key = K_MOUSE;
    return true;
}

int InputReader::parseAltKey(char ch, Key *key) {
    switch (ch) {
        case 9:
            *key = K_ALT_TAB;
            return true;
        case 13:
            *key = K_ALT_ENTER;
            return true;
        case 32 ... 90:
        case 92 ... 127:
            *key = (Key)(K_ALT_SPACE + ch - 32);
            return true;
        default:
            *key = K_UNKNOWN;
            return false;
    }
}

int InputReader::parseEsqSeq(std::string& seq, Key *key) {
    std::map<std::string, Key>::const_iterator it;
    it = ESC_KEY_LOOKUP.find(seq);
    if (it == ESC_KEY_LOOKUP.end()) {
        *key = K_UNKNOWN;
        return false;
    }
    *key = it->second;
    return true;
}

int InputReader::parseUtf8(char ch, Key *key) {
    int length = utf8CharLen(ch);
    if (length < 2) {
        *key = K_UNKNOWN;
        return false;
    }

    m_widechar[0] = ch;

    int i;
    for (i = 1; i < length; i++) {
        if (!hasKey()) {
            *key = K_UNKNOWN;
            return false;
        }
        m_widechar[i] = getch();
        if (!isContinuationByte(m_widechar[i])) {
            *key = K_UNKNOWN;
            return false;
        }
    }
    m_widechar[i] = 0;

    *key = K_UTF8;
    return true;
}

void InputReader::onLoop() {
    Key key;
    bool success = getKey(&key);
    if (success) {
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
}

void InputReader::preOnEvent(EventType type) {
    close(m_pipe[1]);
}

void InputReader::onEvent(std::shared_ptr<Event> event) {
    m_logger.log("received %s", getEventNames()[event->getType()]);
}
