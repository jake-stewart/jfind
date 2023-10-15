#include "../include/item_preview.hpp"
#include "../include/ansi_style.hpp"
#include "../include/logger.hpp"
#include "../include/config.hpp"
#include "../include/util.hpp"
#include <cstring>

void ItemPreview::canOptimizeAnsi(bool value) {
    m_optimizeAnsi = value;
}

bool ItemPreview::scrollUp(int lines) {
    if (lines >= m_offset) {
        if (m_offset == 0) {
            return false;
        }
        m_offset = 0;
        redraw();
        return true;
    }
    else if (!m_optimizeAnsi || lines >= m_height) {
        m_offset -= lines;
        redraw();
        return true;
    }
    for (int i = 0; i < lines; i++) {
        m_offset--;
        AnsiWrapper::instance().setScrollRegion(m_y, m_y + m_height - 1);
        AnsiWrapper::instance().scrollDown();
        printLine(m_y);
    }
    return true;
}

bool ItemPreview::scrollDown(int lines) {
    if (m_content.lines.size() - m_offset + lines <= m_height) {
        if (m_content.lines.size() - m_offset <= m_height) {
            return false;
        }
        m_offset = std::max((int)m_content.lines.size() - m_height, 0);
        redraw();
        return true;
    }
    else if (!m_optimizeAnsi || lines >= m_height) {
        m_offset = std::min((int)m_content.lines.size() - m_height, m_offset + lines);
        redraw();
        return true;
    }
    for (int i = 0; i < lines; i++) {
        m_offset++;
        AnsiWrapper::instance().setScrollRegion(m_y, m_y + m_height - 1);
        AnsiWrapper::instance().scrollUp();
        printLine(m_height - 1);
    }
    return true;
}

void ItemPreview::printLine(int idx) {
    AnsiWrapper::instance().move(m_x, m_y + idx);
    AnsiString &line = m_content.lines[m_offset + idx];
    int remainingLength = m_width;
    bool isFocusLine = m_offset + idx + 1 == m_content.lineNumber;
    for (AnsiSpan span : line.spans) {
        if (span.escapeCode.size()) {
            fprintf(stderr, "%s", span.escapeCode.c_str());
        }
        if (isFocusLine) {
            if (!StyleManager::instance().set(Config::instance().previewLineStyle)) {
                StyleManager::instance().set();
            }
        }
        if (span.content.length() <= remainingLength) {
            fprintf(stderr, "%s", span.content.c_str());
        }
        else {
            fprintf(stderr, "%.*s", remainingLength, span.content.c_str());
        }
        remainingLength -= span.content.length();
        if (remainingLength <= 0) {
            break;
        }
    }

    if (isFocusLine) {
        if (!StyleManager::instance().set(Config::instance().previewLineStyle)) {
            StyleManager::instance().set();
        }
    }
    else {
        fprintf(stderr, "\x1b[0m");
    }
    for (int i = 0; i < remainingLength; i++) {
        fprintf(stderr, " ");
    }
    if (isFocusLine) {
        fprintf(stderr, "\x1b[0m");
    }
}

void ItemPreview::redraw() {
    if (m_height <= 0 || m_width <= 0) {
        return;
    }
    int numLines = std::min(m_height, (int)m_content.lines.size() - m_offset);
    fprintf(stderr, "\x1b[0m");
    for (int i = 0; i < numLines; i++) {
        printLine(i);
    }
    for (int i = numLines; i < m_height; i++) {
        AnsiWrapper::instance().move(m_x, m_y + i);
        for (int i = 0; i < m_width; i++) {
            fprintf(stderr, " ");
        }
    }
    StyleManager::instance().set();
}

void ItemPreview::resize(int x, int y, int w, int h) {
    m_width = w;
    m_height = h;
    m_x = x;
    m_y = y;
    redraw();
}

void ItemPreview::refresh(ItemPreviewContent content) {
    m_offset = 0;
    m_content = content;

    if (m_content.lineNumber <= m_content.lines.size()) {
        m_offset = m_content.lineNumber - m_height / 2;
        // if (m_offset + m_height > m_content.lines.size()) {
        //     m_offset = m_content.lines.size() - m_height;
        // }
        if (m_offset < 0) {
            m_offset = 0;
        }
    }
    redraw();
}
