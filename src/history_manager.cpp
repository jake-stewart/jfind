#include "../include/history_manager.hpp"
#include "../include/util.hpp"
#include "../include/thread_manager.hpp"
#include <fstream>
#include <iostream>

HistoryManager::HistoryManager(fs::path file) {
    m_historyLimit = 20;
    m_file = file;
}

void HistoryManager::setHistoryLimit(int historyLimit) {
    m_historyLimit = historyLimit;
}

bool HistoryManager::readHistory() {
    std::ifstream historyFile(expandUserPath(m_file));
    if (!historyFile.is_open()) {
        return false;
    }

    while (!historyFile.eof()) {
        std::string line;
        if (!getline(historyFile, line)) {
            break;
        }
        if (!m_historyLookup.contains(line)) {
            m_historyLookup[line] = m_historyLookup.size();
        }
    }

    historyFile.close();
    return true;
}

void HistoryManager::applyHistory(Item *item) {
    std::unordered_map<std::string, int>::const_iterator it
        = m_historyLookup.find(item->text);
    if (it != m_historyLookup.end()) {
        item->index = -it->second - 1;
    }
}

bool HistoryManager::writeHistory(Item *selected) {
    fs::path expanded = expandUserPath(m_file);
    fs::path parent = expanded.parent_path();
    if (!parent.empty() && !fs::exists(parent)) {
        try {
            fs::create_directories(parent);
        }
        catch (std::exception) {
            fprintf(stderr,
                    "ERROR: could not create parent directories for '%s'\n",
                    expanded.c_str());
            return false;
        }
    }
    if (fs::exists(expanded) && !fs::is_regular_file(expanded)) {
        fprintf(stderr, "ERROR: '%s' is not a regular file\n",
                expanded.c_str());
        return false;
    }
    std::ofstream historyFile(expanded);

    if (!historyFile.is_open()) {
        fprintf(stderr, "ERROR: '%s' could not be opened\n",
                expanded.c_str());
        return false;
    }

    if (m_historyLookup.contains(selected->text)) {
        m_historyLookup[selected->text] = m_historyLookup.size() - 1;
    }
    else {
        m_historyLookup[selected->text] = m_historyLookup.size();
    }

    int size = m_historyLookup.size();
    std::string history[size];

    for (std::pair<std::string, int> it : m_historyLookup) {
        history[it.second] = it.first;
    }

    int start = size > m_historyLimit ? size - m_historyLimit : 0;

    for (int i = start; i < size; i++) {
        if (!history[i].size()) {
            continue;
        }
        historyFile << history[i] << '\n';
    }

    historyFile.close();

    return true;
}
