#ifndef STYLE_MANAGER_HPP
#define STYLE_MANAGER_HPP

#include "ansi_style.hpp"
#include <map>
#include <vector>

const int NO_STYLE = -1;

class StyleManager
{
public:
    static StyleManager &instance();
    void setOutputFile(FILE *outputFile);
    int add(AnsiStyle &style);
    bool set(int idx);
    void set();

private:
    std::map<std::string, int> m_lookup;
    std::vector<std::string> m_escSeqs;
    int m_currentStyle = NO_STYLE;
    FILE *m_outputFile = stdout;
};

#endif
