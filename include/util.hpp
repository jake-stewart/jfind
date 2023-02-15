#ifndef UTIL_HPP
#define UTIL_HPP

#include <string>
#include <filesystem>
#include <vector>

std::vector<std::string> split(std::string str, char delim);
std::string toLower(std::string s);
std::string& strip(std::string &str);
std::filesystem::path expandUserPath(std::string user_path);
int mod(int n, int m);
bool isVowel(char c);
bool isInteger(const char *str);

#endif
