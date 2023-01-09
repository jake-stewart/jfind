#include "../include/color_rgb.hpp"

bool parseHexColor(std::string& hex, ColorRGB *rgb) {
    int result = 0;

    if (hex.length() == 4) {
        result = sscanf(hex.c_str(), "#%01hhx%01hhx%01hhx",
                &rgb->r, &rgb->g, &rgb->b);

        rgb->r = (rgb->r << 4) + rgb->r;
        rgb->g = (rgb->g << 4) + rgb->g;
        rgb->b = (rgb->b << 4) + rgb->b;
    }
    else if (hex.length() == 7) {
        result = sscanf(hex.c_str(), "#%02hhx%02hhx%02hhx",
            &rgb->r, &rgb->g, &rgb->b);
    }

    return result == 3;
}


