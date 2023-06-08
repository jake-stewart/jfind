#include "../include/utf8_string.hpp"

#include <locale>
#include <codecvt>

static std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

void Utf8StringCursor::setString(Utf8String *str) {
    m_str = str;
}

void Utf8StringCursor::insert(char ch) {
    m_str->cellWidths.insert(m_str->cellWidths.begin() + m_charIdx, 1);
    m_str->byteWidths.insert(m_str->byteWidths.begin() + m_charIdx, 1);
    m_str->bytes.insert(m_str->bytes.begin() + m_byteIdx, ch);
    m_byteIdx += 1;
    m_cellIdx += 1;
    m_charIdx += 1;
}

void Utf8StringCursor::insert(std::string& str) {
    for (int i = 0; i < str.size(); i++) {
        unsigned char ch = str[i];
        if (32 <= ch && ch < 127) {
            insert(str[i]);
        }
        else if (ch > 127) {
            char widechar[5] = {0};
            int widecharIdx = 0;

            int len = utf8CharLen(ch);
            widechar[0] = ch;
            widecharIdx++;
            i++;
            for (int j = 1; j < len; j++) {
                if (!isContinuationByte(str[i])) {
                    break;
                }
                widechar[widecharIdx] = str[i];
                widecharIdx++;
                i++;
            }
            i--;

            std::wstring ws;
            try {
                ws = converter.from_bytes(widechar);
            }
            catch (const std::range_error&) {
                continue;
            }

            for (int j = 0; j < widecharIdx; j++) {
                m_str->bytes.insert(m_str->bytes.begin() + m_byteIdx,
                        widechar[j]);
                m_byteIdx++;
            }
            int width = wcswidth(ws.c_str(), ws.size());
            m_str->cellWidths.insert(m_str->cellWidths.begin() + m_charIdx,
                    width);
            m_str->byteWidths.insert(m_str->byteWidths.begin() + m_charIdx,
                    widecharIdx);
            m_cellIdx += width;
            m_charIdx += 1;
        }
    }
}

bool Utf8StringCursor::moveLeft() {
    if (m_charIdx == 0) {
        return false;
    }
    m_cellIdx -= m_str->cellWidths[--m_charIdx];
    m_byteIdx -= m_str->byteWidths[m_charIdx];
    return true;
}

bool Utf8StringCursor::moveRight() {
    if (m_charIdx == m_str->byteWidths.size()) {
        return false;
    }
    m_cellIdx += m_str->cellWidths[m_charIdx];
    m_byteIdx += m_str->byteWidths[m_charIdx++];
    return true;
}

bool Utf8StringCursor::backspace() {
    if (m_charIdx <= 0) {
        return false;
    }
    m_str->bytes.erase(m_str->bytes.begin() + m_byteIdx
            - m_str->byteWidths[m_charIdx - 1],
            m_str->bytes.begin() + m_byteIdx);
    m_byteIdx -= m_str->byteWidths[m_charIdx - 1];
    m_cellIdx -= m_str->cellWidths[m_charIdx - 1];
    m_str->byteWidths.erase(m_str->byteWidths.begin() + m_charIdx - 1);
    m_str->cellWidths.erase(m_str->cellWidths.begin() + m_charIdx - 1);
    m_charIdx--;
    return true;
}

bool Utf8StringCursor::del() {
    if (m_charIdx >= m_str->byteWidths.size()) {
        return false;
    }
    m_str->bytes.erase(m_str->bytes.begin() + m_byteIdx,
            m_str->bytes.begin() + m_byteIdx + m_str->byteWidths[m_charIdx]);
    m_str->byteWidths.erase(m_str->byteWidths.begin() + m_charIdx);
    m_str->cellWidths.erase(m_str->cellWidths.begin() + m_charIdx);
    return true;
}

void Utf8StringCursor::reset() {
    m_byteIdx = 0;
    m_cellIdx = 0;
    m_charIdx = 0;
}

int Utf8StringCursor::getBytesForCols(int cols) {
    int w = 0;
    int bytes = 0;
    int idx = m_charIdx;

    while (idx < m_str->cellWidths.size() && w + m_str->cellWidths[idx] < cols)
    {
        w += m_str->cellWidths[idx];
        bytes += m_str->byteWidths[idx];
        idx++;
    }

    return bytes;
}

int Utf8StringCursor::getByte() const {
    return m_byteIdx;
}

int Utf8StringCursor::getIdx() const {
    return m_charIdx;
}

int Utf8StringCursor::getCol() const {
    return m_cellIdx;
}

const char* Utf8StringCursor::getPointer() const {
    return m_str->bytes.c_str() + m_byteIdx;
}
