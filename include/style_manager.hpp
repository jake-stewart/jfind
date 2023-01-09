#ifndef STYLE_MANAGER_HPP
#define STYLE_MANAGER_HPP

#include <map>
#include <vector>
#include "ansi_style.hpp"

const int NO_STYLE = -1;

class StyleManager {
    public:
        StyleManager();
        int add(AnsiStyle& style);
        void set(int idx);
        void setOutputFile(FILE *file);

    private:
        std::map<std::string, int> m_lookup;
        std::vector<std::string> m_escSeqs;
        int m_currentStyle = NO_STYLE;
        FILE *m_outputFile;
};

#endif
