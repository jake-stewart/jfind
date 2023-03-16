#ifndef UTF8_LINE_EDITOR_HPP
#define UTF8_LINE_EDITOR_HPP

#include "input_reader.hpp"
#include "utf8_string.hpp"

#include <string>

class Utf8LineEditor {
    public:
        Utf8LineEditor(FILE *outputFile);
        bool requiresRedraw();
        void print();
        void input(char ch);
        void input(std::string text);
        void handleClick(int x);
        void backspace();
        void del();
        void moveCursorLeft();
        void moveCursorRight();
        void clear();
        std::string& getText();
        int getCursorCol();
        void setWidth(int width);

    private:
        Utf8String m_string;
        Utf8StringCursor m_cursor;
        Utf8StringCursor m_start;
        Utf8StringCursor m_end;
        FILE *m_outputFile;
        bool m_requiresRedraw;
        int m_width;

        void adjustBounds();
};

#endif
