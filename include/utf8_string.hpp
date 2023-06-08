#ifndef UTF8_STRING_HPP
#define UTF8_STRING_HPP

#include <string>
#include <vector>

#include "input_reader.hpp"

struct Utf8String {
    std::string bytes;
    std::vector<int> byteWidths;
    std::vector<int> cellWidths;
};

class Utf8StringCursor {
    public:
        int getByte() const;
        int getIdx() const;
        int getCol() const;
        void setString(Utf8String *str);
        void insert(char ch);
        void insert(std::string& str);
        bool moveLeft();
        bool moveRight();
        bool backspace();
        bool del();
        void reset();
        int getBytesForCols(int cols);
        const char* getPointer() const;

    private:
        Utf8String *m_str;
        int m_charIdx = 0;
        int m_byteIdx = 0;
        int m_cellIdx = 0;
};

#endif
