#ifndef ANSI_STRING_HPP
#define ANSI_STRING_HPP

#include <string>
#include <vector>

struct AnsiSpan {
    std::string escapeCode;
    std::string content;
};

struct AnsiString {
    std::vector<AnsiSpan> spans;
    int length;
};

#endif
