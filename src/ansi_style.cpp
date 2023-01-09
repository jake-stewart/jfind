#include "../include/ansi_style.hpp"
#include <sstream>

AnsiStyle& AnsiStyle::fg(Color16 color) {
    if (color >= BRIGHT_BLACK) {
        return fg((int)color);
    }
    m_fgType = COLOR_8;
    m_fg16 = color;
    return *this;
}

AnsiStyle& AnsiStyle::bg(Color16 color) {
    if (color >= BRIGHT_BLACK) {
        return bg((int)color);
    }
    m_bgType = COLOR_8;
    m_bg16 = color;
    return *this;
}

AnsiStyle& AnsiStyle::fg(int color) {
    m_fgType = COLOR_256;
    m_fg256 = color;
    return *this;
}

AnsiStyle& AnsiStyle::bg(int color) {
    m_bgType = COLOR_256;
    m_bg256 = color;
    return *this;
}

AnsiStyle& AnsiStyle::fg(ColorRGB color) {
    m_fgType = COLOR_RGB;
    m_fgRgb = color;
    return *this;
}

AnsiStyle& AnsiStyle::bg(ColorRGB color) {
    m_bgType = COLOR_RGB;
    m_bgRgb = color;
    return *this;
}

AnsiStyle& AnsiStyle::bold() {
    m_bold = true;
    return *this;
}

AnsiStyle& AnsiStyle::italic() {
    m_italic = true;
    return *this;
}

AnsiStyle& AnsiStyle::underline() {
    m_underlineType = LINE;
    return *this;
}

AnsiStyle& AnsiStyle::blink() {
    m_blink = true;
    return *this;
}

AnsiStyle& AnsiStyle::standout() {
    m_standout = true;
    return *this;
}

AnsiStyle& AnsiStyle::strikethrough() {
    m_strikethrough = true;
    return *this;
}

AnsiStyle& AnsiStyle::dim() {
    m_dim = true;
    return *this;
}

AnsiStyle& AnsiStyle::underline(UnderlineType type) {
    m_underlineType = type;
    return *this;
}

std::string AnsiStyle::build() {
    std::stringstream ss;
    ss << "\x1b[0";

    switch (m_fgType) {
        case COLOR_8:
            ss << ";" << (30 + m_fg16);
            break;
        case COLOR_256:
            ss << ";38;5;" << m_fg256;
            break;
        case COLOR_RGB:
            ss << ";38;2;"
                << (int)m_fgRgb.r << ";"
                << (int)m_fgRgb.g << ";"
                << (int)m_fgRgb.b;
            break;
        case NO_COLOR:
            break;
    }

    switch (m_bgType) {
        case COLOR_8:
            ss << ";" << (40 + m_bg16);
            break;
        case COLOR_256:
            ss << ";48;5;" << m_bg256;
            break;
        case COLOR_RGB:
            ss << ";48;2;"
                << (int)m_bgRgb.r << ";"
                << (int)m_bgRgb.g << ";"
                << (int)m_bgRgb.b;
            break;
        case NO_COLOR:
            break;
    }

    if (m_bold) ss << ";1";
    if (m_dim) ss << ";2";
    if (m_italic) ss << ";3";
    if (m_blink) ss << ";5";
    if (m_standout) ss << ";7";
    if (m_strikethrough) ss << ";9";

    if (m_underlineType) {
        ss << ";4";
        switch (m_underlineType) {
            case DOUBLE:
                ss << ":2";
                break;
            case CURLY:
                ss << ":3";
                break;
            case DOTTED:
                ss << ":4";
                break;
            case DASHED:
                ss << ":5";
                break;
            default:
                break;
        }
    }

    ss << "m";
    return ss.str();
}
