#include "../include/utf8_string.hpp"

#include <locale>
#include <codecvt>

std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

void Utf8StringCursor::setString(Utf8String *str) {
    m_str = str;
}

void Utf8StringCursor::insert(char ch) {
    m_str->cell_widths.insert(m_str->cell_widths.begin() + m_char_idx, 1);
    m_str->byte_widths.insert(m_str->byte_widths.begin() + m_char_idx, 1);
    m_str->bytes.insert(m_str->bytes.begin() + m_byte_idx, ch);
    m_byte_idx += 1;
    m_cell_idx += 1;
    m_char_idx += 1;
}

void Utf8StringCursor::insert(std::string& str) {
    for (int i = 0; i < str.size(); i++) {
        unsigned char ch = str[i];
        if (32 <= ch && ch < 127) {
            insert(str[i]);
        }
        else if (ch > 127) {
            char widechar[5] = {0};
            int widechar_idx = 0;

            int len = utf8CharLen(ch);
            widechar[0] = ch;
            widechar_idx++;
            i++;
            for (int j = 1; j < len; j++) {
                if (!isContinuationByte(str[i])) {
                    break;
                }
                widechar[widechar_idx] = str[i];
                widechar_idx++;
                i++;
            }
            i--;

            std::wstring ws;
            try {
                ws = converter.from_bytes(widechar);
            }
            catch (std::range_error) {
                continue;
            }

            for (int j = 0; j < widechar_idx; j++) {
                m_str->bytes.insert(m_str->bytes.begin() + m_byte_idx, widechar[j]);
                m_byte_idx++;
            }
            int width = wcswidth(ws.c_str(), ws.size());
            m_str->cell_widths.insert(m_str->cell_widths.begin() + m_char_idx, width);
            m_str->byte_widths.insert(m_str->byte_widths.begin() + m_char_idx, widechar_idx);
            m_cell_idx += width;
            m_char_idx += 1;
        }
    }
}

bool Utf8StringCursor::moveLeft() {
    if (m_char_idx == 0) {
        return false;
    }
    m_cell_idx -= m_str->cell_widths[--m_char_idx];
    m_byte_idx -= m_str->byte_widths[m_char_idx];
    return true;
}

bool Utf8StringCursor::moveRight() {
    if (m_char_idx == m_str->byte_widths.size()) {
        return false;
    }
    m_cell_idx += m_str->cell_widths[m_char_idx];
    m_byte_idx += m_str->byte_widths[m_char_idx++];
    return true;
}

bool Utf8StringCursor::backspace() {
    if (m_char_idx <= 0) {
        return false;
    }
    m_str->bytes.erase(
        m_str->bytes.begin() + m_byte_idx - m_str->byte_widths[m_char_idx - 1],
        m_str->bytes.begin() + m_byte_idx
    );
    m_byte_idx -= m_str->byte_widths[m_char_idx - 1];
    m_cell_idx -= m_str->cell_widths[m_char_idx - 1];
    m_str->byte_widths.erase(m_str->byte_widths.begin() + m_char_idx - 1);
    m_str->cell_widths.erase(m_str->cell_widths.begin() + m_char_idx - 1);
    m_char_idx--;
    return true;
}

bool Utf8StringCursor::del() {
    if (m_char_idx >= m_str->byte_widths.size()) {
        return false;
    }
    m_str->bytes.erase(
        m_str->bytes.begin() + m_byte_idx,
        m_str->bytes.begin() + m_byte_idx + m_str->byte_widths[m_char_idx]
    );
    m_str->byte_widths.erase(m_str->byte_widths.begin() + m_char_idx);
    m_str->cell_widths.erase(m_str->cell_widths.begin() + m_char_idx);
    return true;
}

void Utf8StringCursor::reset() {
    m_byte_idx = 0;
    m_cell_idx = 0;
    m_char_idx = 0;
}

int Utf8StringCursor::getBytesForCols(int cols) {
    int w = 0;
    int bytes = 0;
    int idx = m_char_idx;

    while (idx < m_str->cell_widths.size() && w + m_str->cell_widths[idx] < cols) {
        w += m_str->cell_widths[idx];
        bytes += m_str->byte_widths[idx];
        idx++;
    }

    return bytes;
}

int Utf8StringCursor::getByte() {
    return m_byte_idx;
}

int Utf8StringCursor::getIdx() {
    return m_char_idx;
}

int Utf8StringCursor::getCol() {
    return m_cell_idx;
}

const char* Utf8StringCursor::getPointer() {
    return m_str->bytes.c_str() + m_byte_idx;
}
