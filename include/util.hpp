#ifndef UTIL_HPP
#define UTIL_HPP

#include <string>
#include <vector>
#include <istream>
#include <filesystem>
#include <set>

#include "option.hpp"

namespace fs = std::filesystem;

std::vector<std::string>      split(std::string str, char delim);
std::vector<std::string>      camelSplit(std::string& text);
std::vector<std::string_view> camelSplitLower(std::string_view text);
std::string              toLower(std::string s);
std::string&             strip(std::string &str);
void                     printUsage();

bool                     getNonEmptyLine(std::istream& is,
                                         std::string& line);

fs::path                 expandUserPath(std::string user_path);

int mod(int n, int m);
bool isVowel(char c);
bool is_lower(char c);
bool is_number(char c);
bool is_upper(char c);
bool is_alnum(char c);
bool is_lower_alnum(char c);

char fast_tolower(char c);
bool fast_isalpha(char c);
bool fast_islower(char c);
bool fast_isdigit(char c);

#endif
