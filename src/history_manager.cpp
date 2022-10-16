#include "../include/history_manager.hpp"
#include "../include/util.hpp"
#include <fstream>
#include <iostream>
#include <map>

HistoryManager::HistoryManager(fs::path file) {
    m_file = file;
}

int HistoryManager::historyCount;

bool HistoryManager::createHistoryLookup() {
    std::ifstream history_file(expandUserPath(m_file));
    if (!history_file.is_open()) {
        return false;
    }

    while (!history_file.eof()) {
        std::string line;
        int history;
        history_file >> history;
        if (!getNonEmptyLine(history_file, line)) {
            break;
        }
        m_history_lookup[line] = history;
    }

    history_file.close();
    return true;
}

bool HistoryManager::readHistory(Sources& sources) {
    if (!createHistoryLookup()) {
        return false;
    }

    for (int i = 0; i < sources.size(); i++) {
        Source& source = sources.get(i);
        int history = m_history_lookup[source.getName()];
        source.setHistory(history);
    }

    return true;
}

bool HistoryManager::writeHistory(Source& selected, Sources& sources) {
    selected.setHistory(historyCount + 1);
    fs::path expanded = expandUserPath(m_file);
    fs::path parent = expanded.parent_path();
    if (!parent.empty()) {
        fs::create_directories(parent);
    }
    std::ofstream history_file(expanded);

    if (!history_file.is_open()) {
        return false;
    }

    for (int i = 0; i < sources.size(); i++) {
        Source& source = sources.get(i);
        int history = source.getHistory() - 1;
        if (history > 0) {
            if (history > historyCount) {
                history = historyCount;
            }
            history_file << history << " ";
            history_file << source.getName() << std::endl;
        }
    }

    history_file.close();

    return true;
}
