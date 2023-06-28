#ifndef CHOICE_HPP
#define CHOICE_HPP

#include <string>

template <typename T>
struct Choice {
    std::string name;
    T value;
};

#endif
