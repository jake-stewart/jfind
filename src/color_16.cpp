#include "../include/color_16.hpp"
#include "../include/util.hpp"

Color16 parseColor16(std::string name) {
    name = toLower(name);

    bool bright = false;
    if (name.starts_with("bright_")) {
        bright = true;
        name = name.substr(7);
    }

    Color16 color = UNKNOWN;

    if (name == "red") {
        color = RED;
    }
    else if (name == "green") {
        color = GREEN;
    }
    else if (name == "blue") {
        color = BLUE;
    }
    else if (name == "cyan") {
        color = CYAN;
    }
    else if (name == "yellow") {
        color = YELLOW;
    }
    else if (name == "magenta" || name == "purple") {
        color = MAGENTA;
    }
    else if (name == "black") {
        color = BLACK;
    }
    else if (name == "white") {
        color = WHITE;
    }
    else {
        return UNKNOWN;
    }

    if (bright) {
        return (Color16)(color + 8);
    }
    return color;
}
