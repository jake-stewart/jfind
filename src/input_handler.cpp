#include "../include/input_handler.hpp"
#include "../include/input_reader.hpp"
#include "../include/util.hpp"
#include "../include/key.hpp"

bool InputHandler::isActive() {
    return m_active;
}

bool InputHandler::isSelected() {
    return m_selected;
}

int InputHandler::getOffset() {
    return m_offset;
}

std::string InputHandler::getQuery() {
    return m_query;
}

void InputHandler::setQuery(std::string query) {
    m_query = query;
}

void InputHandler::scrollOffset(int delta) {
    if (m_offset_range) {
        m_offset = mod(m_offset + delta, m_offset_range);
    }
}

void InputHandler::setOffsetRange(int offset_range) {
    m_offset_range = offset_range;
}

int InputHandler::getCursorPos() {
    return m_cursor_pos;
}

void InputHandler::moveCursor(int delta) {
    m_cursor_pos += delta;
    if (m_cursor_pos <= 0) {
        m_cursor_pos = 0;
    }
    else if (m_cursor_pos >= m_query.size()) {
        m_cursor_pos = m_query.size();
    }
}

InputReader reader;

void InputHandler::handle() {

    Key key;
    reader.getKey(&key);

    switch (key) {
        case K_ESCAPE:
        case K_CTRL_C:
            m_active = false;
            break;

        // match every keyboard character including symbols
        case 32 ... 126:
            m_offset = 0;
            m_query.insert(m_query.begin() + m_cursor_pos, key);
            m_cursor_pos += 1;
            break;

        case K_BACKSPACE:
        case K_CTRL_H:
            m_offset = 0;
            if (m_cursor_pos - 1 < m_query.size() && m_cursor_pos - 1 >= 0) {
                m_query.erase(m_query.begin() + m_cursor_pos - 1);
                m_cursor_pos -= 1;
            }
            break;
        
        case K_DELETE:
            m_offset = 0;
            if (m_cursor_pos < m_query.size() && m_cursor_pos >= 0) {
                m_query.erase(m_query.begin() + m_cursor_pos);
            }
            break;

        case K_CTRL_U:
            m_offset = 0;
            m_cursor_pos = 0;
            m_query[0] = '\0';
            break;

        case K_UP:
        case K_CTRL_K:
            scrollOffset(1);
            break;

        case K_DOWN:
        case K_CTRL_J:
            scrollOffset(-1);
            break;

        case K_LEFT:
            moveCursor(-1);
            break;

        case K_RIGHT:
            moveCursor(1);
            break;

        case K_ENTER:
            m_selected = true;
            m_active = false;
            break;

        case K_UTF8: {
            /* std::string glyph = reader.getWideChar(); */
            /* for (int i = 0; i < glyph.size(); i++) { */
            /*     m_query.pu */
            /*     m_query[m_query_size++] = glyph[i]; */
            /* } */
            break;
         }

        default:
            break;
    }
}
