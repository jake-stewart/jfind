#include "../include/style_manager.hpp"

StyleManager::StyleManager(FILE *outputFile) {
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

void StyleManager::set(int idx) {
    if (idx == m_currentStyle) {
        return;
    }
    if (idx == NO_STYLE) {
        fprintf(m_outputFile, "\x1b[0m");
    }
    else {
        fprintf(m_outputFile, "%s", m_escSeqs[idx].c_str());
    }
    m_currentStyle = idx;
}
