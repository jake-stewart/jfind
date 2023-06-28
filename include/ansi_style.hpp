#ifndef ANSI_STYLE_HPP
#define ANSI_STYLE_HPP

#include "color_16.hpp"
#include "color_rgb.hpp"
#include <string>

enum UnderlineType {
    NO_UNDERLINE,
    LINE,
    DOUBLE,
    CURLY,
    DOTTED,
    DASHED
};

enum ColorType {
    NO_COLOR,
    COLOR_RGB,
    COLOR_8,
    COLOR_256,
};

class AnsiStyle
{
public:
    AnsiStyle &blend();
    AnsiStyle &fg(int color);
    AnsiStyle &bg(int color);
    AnsiStyle &fg(ColorRGB color);
    AnsiStyle &bg(ColorRGB color);
    AnsiStyle &bold();
    AnsiStyle &italic();
    AnsiStyle &underline();
    AnsiStyle &blink();
    AnsiStyle &standout();
    AnsiStyle &strikethrough();
    AnsiStyle &dim();
    AnsiStyle &underline(UnderlineType type);
    std::string build() const;

private:
    ColorType m_fgType = NO_COLOR;
    ColorRGB m_fgRgb;
    Color16 m_fg16;
    int m_fg256;

    ColorType m_bgType = NO_COLOR;
    ColorRGB m_bgRgb;
    Color16 m_bg16;
    int m_bg256;

    bool m_blend = false;
    bool m_bold = false;
    bool m_italic = false;
    bool m_blink = false;
    bool m_standout = false;
    bool m_strikethrough = false;
    bool m_dim = false;

    UnderlineType m_underlineType = NO_UNDERLINE;
};

#endif
