#ifndef COLOR_16_HPP
#define COLOR_16_HPP

#include <string>

enum Color16
{
    UNKNOWN = -1,

    BLACK,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN,
    WHITE,

    BRIGHT_BLACK,
    BRIGHT_RED,
    BRIGHT_GREEN,
    BRIGHT_YELLOW,
    BRIGHT_BLUE,
    BRIGHT_MAGENTA,
    BRIGHT_CYAN,
    BRIGHT_WHITE,
};

Color16 parseColor16(std::string name);

#endif
