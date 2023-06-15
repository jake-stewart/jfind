#include "../include/util.hpp"

#include <vector>

namespace fs = std::filesystem;

std::vector<std::string> split(std::string str, char delim) {
    std::vector<std::string> words;
    std::string word = "";

    for(char c : str) {
        if(c == delim){
           words.push_back(word);
           word = "";
        }
        else {
           word += c;
        }
    }
    if (word.size() > 0) {
        words.push_back(word);
    }
    return words;
}

std::string toLower(std::string s) {
    for (char &c : s) {
        c = tolower(c);
    }
    return s;
}

fs::path expandUserPath(std::string userPath) {
    if (userPath == "~") {
        return getenv("HOME");
    }
    if (userPath.starts_with("~/")) {
        return getenv("HOME") / fs::path(userPath.substr(2));
    }
    return userPath;
}

int mod(int n, int m) {
    return ((n % m) + m) % m;
}

bool isVowel(char c) {
    switch (tolower(c)) {
        case 'a':
        case 'e':
        case 'i':
        case 'o':
        case 'u':
            return true;
        default:
            return false;
    }
}

bool isInteger(const char *str) {
    if (!str || !*str) {
        return false;
    }
    if (*str == '-') {
        str++;
    }
    while (*str) {
        if (!isdigit(*str)) {
            return false;
        }
        str++;
    }
    return true;
}

void replace(std::string& str, char from, char to) {
    for (size_t i = 0; i < str.length(); i++) {
        if (str[i] == from) {
            str[i] = to;
        }
    }
}
