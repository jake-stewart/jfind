#include "../include/source.hpp"
#include "../include/util.hpp"
#include "../include/history_manager.hpp"
#include <cstdlib>

void Source::setHeuristic(int heuristic) {
    m_heuristic = heuristic;
}

void Source::calcHeuristic(
        std::string& query,
        std::vector<std::string>& words
) {
    m_heuristic = m_name.size() - (m_history * HistoryManager::historyCount);

    int pos = m_lower_name.find(query);
    if (pos != std::string::npos) {
        if (pos == 0) {
            m_heuristic -= 50;
        }
        m_heuristic -= 100;
    }

    else if (words.size()) {
        int j = 0;
        int i = 0;
        for (i = 0; i < m_words.size(); i++) {
            int pos = m_words[i].find(words[j]);
            if (pos == std::string::npos) {
                continue;
            }
            j++;
            if (j == words.size()) {
                break;
            }
        }
        if (j != words.size() || i == m_words.size()) {
            m_heuristic = INT_MAX;
            return;
        }
    }

    int last_j = 0;

    for (int i = 0; i < words.size(); i++) {
        float max_heuristic = 0;
        for (int j = last_j; j < m_words.size(); j++) {
            int pos = m_words[j].find(words[i]);
            if (pos != std::string::npos) {
                last_j = j;
                float heuristic = 1;
                if (pos == 0) {
                    heuristic += 200;
                }
                heuristic = (
                    (heuristic * 200.0) // * (1.0 + (comp_word.size()) * 0.5)
                );
                // heuristic -= std::abs(j-i) * 5;
                if (heuristic < 0) {
                    heuristic = 0;
                }
                if (heuristic > max_heuristic) {
                    max_heuristic = heuristic;
                }
            }
        }
        m_heuristic -= max_heuristic;
    }
}

int Source::getHeuristic() {
    return m_heuristic;
}

Source::Source() {
    m_history = 0;
}

void Source::setName(std::string name) {
    m_name = name;
    m_lower_name = toLower(m_name);
    m_words = camelSplitLower(m_name, m_lower_name);
}

std::string& Source::getName() {
    return m_name;
}

void Source::setPath(std::string src_path) {
    m_path = src_path;
}

std::string& Source::getPath() {
    return m_path;
}

void Source::setHistory(int history) {
    m_history = history;
}

int Source::getHistory() {
    return m_history;
}
