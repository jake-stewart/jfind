#include "../include/ansi_string_parser.hpp"

static int escapeCodeLength(const char *c) {
    if (*c++ != '\x1b' || *c++ != '[') {
        return 0;
    }
    int length = 3;

    while (*c) {
        if (!isnumber(*c) && *c != ';' && *c != ':') {
            break;
        }
        length++;
        c++;
    }

    if (*c != 'm') {
        return 0;
    }

    return length;
}

void AnsiStringParser::setTabstop(int tabstop) {
    m_tabstop = tabstop;
}

AnsiString AnsiStringParser::parse(const char *c) {
    m_string = AnsiString();

    while (*c) {
        if (*c < 32) {
            if (*c == '\x1b') {
                if (m_content.size()) {
                    addSpan();
                }

                int length = escapeCodeLength(c);
                if (length > 0) {
                    m_escapeCode += std::string(c, length);
                    c += length;
                }
                else {
                    c++;
                }
            }
            else if (*c == '\t') {
                for (int i = 0; i < m_tabstop; i++) {
                    m_content += " ";
                }
                c++;
            }
            else {
                m_content.push_back('^');
                m_content.push_back(*c + 64);
                c++;
            }
        }
        else {
            m_content += *c;
            c++;
        }
    }

    if (m_content.size()) {
        addSpan();
    }

    return m_string;
}

void AnsiStringParser::addSpan() {
    AnsiSpan span;
    span.content = m_content;
    span.escapeCode = m_escapeCode;
    m_string.spans.push_back(span);
    m_content = "";
    m_escapeCode = "";
}

