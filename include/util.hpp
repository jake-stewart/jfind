#ifndef UTIL_HPP
#define UTIL_HPP

#include <filesystem>
#include <string>
#include <vector>

std::vector<std::string> split(std::string str, char delim);
std::string toLower(std::string s);
std::string &strip(std::string &str);
std::filesystem::path expandUserPath(std::string user_path);
int mod(int n, int m);
bool isVowel(char c);
bool isInteger(const char *str);
void replace(std::string &str, char from, char to);
void displayHelp(const char *name);

std::string fileName(std::string filePath);
std::string fileStem(std::string filePath);

template <typename T>
void removeConsecutiveDuplicates(std::vector<T> &vec) {
    auto it = vec.begin();
    while (it != vec.end()) {
        auto next = it + 1;
        if (next != vec.end() && *it == *next) {
            vec.erase(next);
        }
        else {
            ++it;
        }
    }
}

#endif
