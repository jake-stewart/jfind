#ifndef OPTION_HPP
#define OPTION_HPP

#include <string>
#include <vector>

struct Option {
    std::string key;
    std::string value;
    bool        hasValue = false;
};

std::vector<Option> parseOptions(int argc, const char **argv);

#endif
