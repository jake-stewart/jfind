#ifndef COLOR_RGB_HPP
#define COLOR_RGB_HPP

#include <string>

struct ColorRGB
{
    unsigned char r = 0;
    unsigned char g = 0;
    unsigned char b = 0;
};

bool parseHexColor(std::string& hex, ColorRGB *rgb);

#endif
