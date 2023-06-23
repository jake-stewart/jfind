#ifndef HISTORY_MANAGER_HPP
#define HISTORY_MANAGER_HPP

#include "item.hpp"
#include <string>
#include <unordered_map>
#include <vector>

class HistoryManager
{
public:
    bool getEntry(int index, std::string **entry);
    bool writeHistory(std::string query);
    static HistoryManager &instance();

private:
    HistoryManager();
    bool readHistory();
    bool m_readHistory;
    std::vector<std::string> m_history;
};

#endif
