#include "../include/history_manager.hpp"
#include "../include/thread_manager.hpp"
#include "../include/logger.hpp"
#include "../include/util.hpp"
#include "../include/config.hpp"
#include <fstream>
#include <iostream>
#include <filesystem>

HistoryManager &HistoryManager::instance() {
    static HistoryManager singleton;
    return singleton;
}

HistoryManager::HistoryManager() {
    m_readHistory = false;
}

bool HistoryManager::getEntry(int index, std::string **entry) {
    if (!m_readHistory) {
        readHistory();
    }
    if (index < 0 || index >= m_history.size()) {
        return false;
    }
    *entry = &m_history[index];
    return true;
}

bool HistoryManager::readHistory() {
    m_readHistory = true;
    fs::path path = Config::instance().historyFile;
    if (path.empty()) {
        return false;
    }

    std::ifstream historyFile(expandUserPath(path));
    if (!historyFile.is_open()) {
        LOG("open '%s' failed", path.c_str());
        return false;
    }

    while (!historyFile.eof()) {
        std::string line;
        if (!getline(historyFile, line)) {
            break;
        }
        m_history.push_back(line);
    }

    historyFile.close();
    return true;
}

bool HistoryManager::writeHistory(std::string query) {
    if (!m_readHistory) {
        if (!readHistory()) {
            return false;
        }
    }
    fs::path path = Config::instance().historyFile;
    if (path.empty()) {
        return false;
    }
    fs::path expanded = expandUserPath(path);
    fs::path parent = expanded.parent_path();
    if (!parent.empty() && !fs::exists(parent)) {
        try {
            fs::create_directories(parent);
        }
        catch (const std::exception &) {
            LOG("create_directories(%s) failed", expanded.c_str());
            return false;
        }
    }
    if (fs::exists(expanded) && !fs::is_regular_file(expanded)) {
        LOG("'%s' exists but is not a regular file", expanded.c_str());
        return false;
    }
    std::ofstream historyFile(expanded);

    if (!historyFile.is_open()) {
        LOG("open '%s' failed", expanded.c_str());
        return false;
    }

    m_history.push_back(query);
    removeConsecutiveDuplicates(m_history);

    int start = m_history.size() > Config::instance().historyLimit
        ? m_history.size() - Config::instance().historyLimit
        : 0;

    for (int i = m_history.size() - 1; i >= start; i--) {
        if (!m_history[i].size()) {
            continue;
        }
        historyFile << m_history[i] << '\n';
    }

    historyFile.close();
    return true;
}
