#include "../include/color_rgb.hpp"

bool parseHexColor(std::string& hex, ColorRGB *rgb) {
    int result;

    if (hex.length() == 3) {
        result = sscanf(
            hex.c_str(),
            "%01hhx%01hhx%01hhx",
            &rgb->r, &rgb->g, &rgb->b
        );

        if (result != 3) {
            return false;
        }

        rgb->r = (rgb->r << 4) + rgb->r;
        rgb->g = (rgb->g << 4) + rgb->g;
        rgb->b = (rgb->b << 4) + rgb->b;
        return true;
    }

    result = sscanf(
        hex.c_str(),
        "%02hhx%02hhx%02hhx",
        &rgb->r, &rgb->g, &rgb->b
    );

    return result == 3;
}


