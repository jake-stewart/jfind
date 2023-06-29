#ifndef ANSI_STRING_PARSER
#define ANSI_STRING_PARSER

#include "ansi_string.hpp"

class AnsiStringParser
{
public:
    AnsiString parse(const char *buffer);
    void setTabstop(int tabstop);

private:
    void addSpan();

    int m_tabstop;
    std::string m_content;
    std::string m_escapeCode;
    AnsiString m_string;
};

#endif
