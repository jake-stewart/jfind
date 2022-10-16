#include "../include/sources.hpp"
#include "../include/util.hpp"

#include <fstream>

bool compareSources(Source& l, Source& r) {
    return (l.getHeuristic() < r.getHeuristic());
}

bool compareSourcesAlpha(Source& l, Source& r) {
    return (l.getName() > r.getName());
}

bool Sources::hasHeuristic() {
    return m_has_heuristic;
}

void Sources::resetHeuristics() {
    for (Source& source : m_sources) {
        source.setHeuristic(0);
    }
}

void Sources::sort(std::string query) {
    if (query.size() < m_last_query_size) {
        resetHeuristics();
    }
    m_last_query_size = query.size();

    std::string lower = toLower(query);
    std::vector<std::string> words = camelSplit(lower);

    if (m_has_heuristic) {
        for (int i = 0; i < m_sources.size(); i++) {
            if (m_sources[i].getHeuristic() == INT_MAX) {
                break;
            }
            m_sources[i].calcHeuristic(lower, words);
        }
    }
    else {
        for (Source& source : m_sources) {
            source.calcHeuristic(lower, words);
        }
    }
    std::sort(m_sources.begin(), m_sources.end(), compareSources);
    m_has_heuristic = true;
}

void Sources::read(std::istream& stream, bool hints) {
    std::string line;
    while (getline(stream, line)) {
        Source source;
        source.setName(line);
        if (hints && getline(stream, line)) {
            source.setPath(line);
        }
        m_sources.push_back(source);
    };
}

bool Sources::readFile(fs::path path, bool hints) {
    std::ifstream file;
    file.open(expandUserPath(path));
    if (!file.is_open()) {
        printf("Error reading file '%s'\n", path.c_str());
        return false;
    }
    read(file, hints);
    file.close();
    return true;
}

size_t Sources::size() {
    return m_sources.size();
}

Source& Sources::get(int idx) {
    return m_sources[idx];
}

void Sources::push_back(Source& source) {
    return m_sources.push_back(source);
}
