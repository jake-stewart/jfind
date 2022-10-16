#ifndef UTIL_HPP
#define UTIL_HPP

#include <string>
#include <vector>
#include <istream>
#include <filesystem>
#include <set>

#include "option.hpp"

namespace fs = std::filesystem;

std::vector<std::string> split(std::string str, char delim);
std::vector<std::string> camelSplit(std::string& text);
std::vector<std::string> camelSplitLower(std::string& text,
                                         std::string& lower);
std::string              toLower(std::string s);
std::string&             strip(std::string &str);
void                     printUsage();

bool                     getNonEmptyLine(std::istream& is,
                                         std::string& line);

void                     recursiveGetFiles(fs::path               root,
                                           std::vector<fs::path>& paths,
                                           std::set<fs::path>&    ignores);

bool                     findPathRootInFile(std::string target,
                                            std::string& root,
                                            std::istream& file);
fs::path                 expandUserPath(std::string user_path);

int                      mod(int n, int m);

#endif
