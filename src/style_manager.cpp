#include "../include/style_manager.hpp"
#include "../include/logger.hpp"

StyleManager &StyleManager::instance() {
    static StyleManager singleton;
    return singleton;
}

void StyleManager::setOutputFile(FILE *outputFile) {
    m_outputFile = outputFile;
}

int StyleManager::add(AnsiStyle &style) {
    std::string escSeq = style.build();
    std::map<std::string, int>::const_iterator it;

    it = m_lookup.find(escSeq);
    if (it != m_lookup.end()) {
        return it->second;
    }
    m_escSeqs.push_back(escSeq);
    return m_escSeqs.size() - 1;
}

bool StyleManager::set(int idx) {
    if (idx == m_currentStyle) {
        return false;
    }
    m_currentStyle = idx;
    set();
    return true;
}

void StyleManager::set() {
    if (m_currentStyle == NO_STYLE) {
        fprintf(m_outputFile, "\x1b[0m");
    }
    else {
        // std::string escSeq = m_escSeqs[m_currentStyle];
        // for (int i = 0; i < escSeq.size(); i++) {
        //     if (escSeq[i] == '\x1b') {
        //         escSeq[i] = 'e';
        //     }
        // }
        // LOG("setting style %s", escSeq.c_str());
        fprintf(m_outputFile, "%s", m_escSeqs[m_currentStyle].c_str());
    }
}
