#include "../include/key.hpp"

std::string getKeyName(Key key) {
    switch (key) {
        case K_ENTER:
            return "ENTER";
        case K_TAB:
            return "TAB";

        case K_CTRL_A ...(K_TAB - 1):
        case (K_TAB + 1)...(K_ENTER - 1):
        case (K_ENTER + 1)... K_CTRL_Z:
            return std::string("CTRL-") + (char)(K_A + key - 1) + "'";

        case K_ESCAPE:
            return "ESCAPE";
        case K_FS:
            return "FS";
        case K_CTRL_CLOSE_BRACKET:
            return "CTRL-]";
        case K_CTRL_BACKSLASH:
            return "CTRL-\\";
        case K_CTRL_SLASH:
            return "CTRL-/";

        case K_SPACE ... K_TILDE:
            return std::string("'") + (char)key + "'";

        case K_BACKSPACE:
            return "BACKSPACE";

        case K_ALT_BACKSPACE:
            return "ALT-BACKSPACE";

        case K_DELETE:
            return "DELETE";

        case K_ALT_TAB:
            return "ALT-TAB";
        case K_ALT_ENTER:
            return "ALT-ENTER";
        case K_ALT_SPACE ... K_ALT_TILDE:
            return std::string("ALT-'") + (char)(K_SPACE + key - K_ALT_SPACE) +
                "'";

        case K_UP:
            return "UP";
        case K_DOWN:
            return "DOWN";
        case K_RIGHT:
            return "RIGHT";
        case K_LEFT:
            return "LEFT";

        case K_SHIFT_UP:
            return "SHIFT UP";
        case K_SHIFT_DOWN:
            return "SHIFT DOWN";
        case K_SHIFT_RIGHT:
            return "SHIFT RIGHT";
        case K_SHIFT_LEFT:
            return "SHIFT LEFT";

        case K_CTRL_UP:
            return "CTRL-UP";
        case K_CTRL_DOWN:
            return "CTRL-DOWN";
        case K_CTRL_RIGHT:
            return "CTRL-RIGHT";
        case K_CTRL_LEFT:
            return "CTRL-LEFT";

        case K_CTRL_TAB:
            return "CTRL-TAB";
        case K_CTRL_SHIFT_TAB:
            return "CTRL-SHIFT-TAB";

        case K_ALT_UP:
            return "ALT-UP";
        case K_ALT_DOWN:
            return "ALT-DOWN";
        case K_ALT_RIGHT:
            return "ALT-RIGHT";
        case K_ALT_LEFT:
            return "ALT-LEFT";

        case K_ALT_SHIFT_TAB:
            return "ALT-SHIFT-TAB";

        case K_F1 ... K_F12:
            return std::string("F") + std::to_string(key - K_F1 + 1);

        default:
            break;
    }
    return "UNKNOWN";
}
