#ifndef STYLE_MANAGER_HPP
#define STYLE_MANAGER_HPP

#include <map>
#include <vector>
#include "ansi_style.hpp"

const int NO_STYLE = -1;

class StyleManager
{
private:
    std::map<std::string, int> m_lookup;
    std::vector<std::string> m_esc_seqs;
    int m_current_style = NO_STYLE;

public:
    int add(AnsiStyle& style) {
        std::string esc_seq = style.build();
        std::map<std::string, int>::const_iterator it;

        it = m_lookup.find(esc_seq);
        if (it != m_lookup.end()) {
            return it->second;
        }
        m_esc_seqs.push_back(esc_seq);
        return m_esc_seqs.size() - 1;
    }

    void set(int idx) {
        if (idx == m_current_style) {
            return;
        }
        if (idx == NO_STYLE) {
            printf("\x1b[0m");
        }
        else {
            printf("%s", m_esc_seqs[idx].c_str());
        }
        m_current_style = idx;
    }
};

#endif
