#ifndef UTF8_LINE_EDITOR_HPP
#define UTF8_LINE_EDITOR_HPP

#include "input_reader.hpp"
#include "utf8_string.hpp"

#include <string>

class Utf8LineEditor
{
private:
    Utf8String m_string;
    Utf8StringCursor m_cursor;
    Utf8StringCursor m_start;
    Utf8StringCursor m_end;

    bool m_modified;
    bool m_redraw;

    int m_width;

    void adjustBounds();

public:
    Utf8LineEditor() {
        m_cursor.setString(&m_string);
        m_start.setString(&m_string);
        m_end.setString(&m_string);
    }

    bool requiresRedraw();
    void print();

    bool isModified();
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
};

#endif
