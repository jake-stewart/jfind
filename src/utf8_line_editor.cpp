#include "../include/utf8_line_editor.hpp"
#include "../include/ansi_wrapper.hpp"
#include "../include/input_reader.hpp"

Utf8LineEditor::Utf8LineEditor(FILE *file) {
    m_outputFile = file;
    m_cursor.setString(&m_string);
    m_start.setString(&m_string);
    m_end.setString(&m_string);
}

void Utf8LineEditor::input(char ch) {
    m_requiresRedraw = true;
    m_cursor.insert(ch);
}

void Utf8LineEditor::input(std::string text) {
    m_requiresRedraw = true;
    m_cursor.insert(text);
    m_end.reset();
}

void Utf8LineEditor::moveCursorLeft() {
    m_requiresRedraw = true;
    m_cursor.moveLeft();
}

void Utf8LineEditor::moveCursorRight() {
    m_requiresRedraw = true;
    m_cursor.moveRight();
}

void Utf8LineEditor::backspace() {
    if (m_cursor.backspace()) {
        m_requiresRedraw = true;
        m_end.reset();
    }
}

void Utf8LineEditor::del() {
    if (m_cursor.del()) {
        m_requiresRedraw = true;
        m_end.reset();
    }
}

void Utf8LineEditor::clear() {
    m_requiresRedraw = true;
    m_string.byteWidths.clear();
    m_string.cellWidths.clear();
    m_string.bytes.clear();
    m_cursor.reset();
    m_start.reset();
    m_end.reset();
}

const std::string &Utf8LineEditor::getText() const {
    return m_string.bytes;
}

void Utf8LineEditor::adjustBounds() {
    // ensure start and end are before and after cursor
    if (m_start.getCol() > m_cursor.getCol()) {
        m_start = m_cursor;
    }
    if (m_end.getCol() < m_cursor.getCol()) {
        m_end = m_cursor;
    }

    // adjust start bound so that cursor is not out of view
    while (m_cursor.getCol() - m_start.getCol() >= m_width &&
           m_start.moveRight()) {
        ;
    }

    // adjust end bound so the line fits as much text as possible
    while (m_start.getCol() + m_width > m_end.getCol() && m_end.moveRight()) {
        ;
    }
    while (m_start.getCol() + m_width < m_end.getCol() && m_end.moveLeft()) {
        ;
    }
}

bool Utf8LineEditor::requiresRedraw() const {
    return m_requiresRedraw;
}

void Utf8LineEditor::print() {
    m_requiresRedraw = false;
    adjustBounds();
    int bytes = m_end.getPointer() - m_start.getPointer();
    fprintf(m_outputFile, "%.*s", bytes, m_start.getPointer());
}

void Utf8LineEditor::handleClick(int x) {
    m_cursor = m_start;
    while (m_cursor.getCol() - m_start.getCol() < x) {
        if (!m_cursor.moveRight()) {
            return;
        }
    }
    m_cursor.moveLeft();
}

void Utf8LineEditor::setWidth(int width) {
    m_width = width - 1;
}

int Utf8LineEditor::getCursorCol() const {
    return m_cursor.getCol() - m_start.getCol();
}
