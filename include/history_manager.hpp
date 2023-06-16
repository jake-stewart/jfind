#ifndef HISTORY_MANAGER_HPP
#define HISTORY_MANAGER_HPP

#include "item.hpp"
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;

class HistoryManager
{
public:
    HistoryManager(fs::path file);
    void setHistoryLimit(int historyLimit);
    bool readHistory();
    void applyHistory(Item *item);
    bool writeHistory(Item *selected);

private:
    std::unordered_map<std::string, int> m_historyLookup;
    int m_historyLimit;
    fs::path m_file;
};

#endif
