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

class AnsiStyle {
    private:
        ColorType     m_fg_type = NO_COLOR;
        ColorRGB      m_fg_rgb;
        Color16       m_fg_16;
        int           m_fg_256;

        ColorType     m_bg_type = NO_COLOR;
        ColorRGB      m_bg_rgb;
        Color16       m_bg_16;
        int           m_bg_256;

        bool          m_bold = 0;
        bool          m_italic = 0;
        bool          m_blink = 0;
        bool          m_standout = 0;
        bool          m_strikethrough = 0;
        bool          m_dim = 0;
        UnderlineType m_underline = NO_UNDERLINE;

    public:
        AnsiStyle& fg(Color16 color);
        AnsiStyle& bg(Color16 color);
        AnsiStyle& fg(int color);
        AnsiStyle& bg(int color);
        AnsiStyle fg(ColorRGB color);
        AnsiStyle bg(ColorRGB color);
        AnsiStyle& bold();
        AnsiStyle& italic();
        AnsiStyle& underline();
        AnsiStyle& blink();
        AnsiStyle& standout();
        AnsiStyle& strikethrough();
        AnsiStyle& dim();
        AnsiStyle& underline(UnderlineType type);
        std::string build();
};

#endif
