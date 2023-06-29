#ifndef UTF8_LINE_EDITOR_HPP
#define UTF8_LINE_EDITOR_HPP

#include "input_reader.hpp"
#include "utf8_string.hpp"

#include <string>

class Utf8LineEditor
{
public:
    Utf8LineEditor(FILE *outputFile);
    bool requiresRedraw() const;
    void redraw();
    void input(char ch);
    void input(std::string text);
    void handleClick(int x);
    bool backspace();
    void backspaceWord();
    void del();
    void moveCursorLeft();
    void moveCursorRight();
    void clear();
    const std::string &getText() const;
    int getCursorCol() const;
    void resize(int x, int y, int width);
    void moveCursorStartOfLine();
    void moveCursorEndOfLine();
    void focus();

private:
    Utf8String m_string;
    Utf8StringCursor m_cursor;
    Utf8StringCursor m_start;
    Utf8StringCursor m_end;
    FILE *m_outputFile;
    bool m_requiresRedraw;

    int m_x;
    int m_y;
    int m_width;

    void adjustBounds();
};

#endif
