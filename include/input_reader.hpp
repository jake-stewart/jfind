#ifndef INPUT_READER_HPP
#define INPUT_READER_HPP

#include "key.hpp"
#include <string>
#include <sys/select.h>

bool isContinuationByte(unsigned char ch);
int utf8CharLen(unsigned char ch);

class InputReader {
    public:
        int getKey(Key *key);
        bool hasKey();
        std::string getWideChar();
        InputReader();

    private:
        fd_set m_set;
        struct timeval m_timeout;
        char m_widechar[4];

        char getch();
        int parseEsc(Key *key);
        int parseAltKey(char ch, Key *key);
        int parseEsqSeq(std::string& seq, Key *key);
        int parseUtf8(char ch, Key *key);

};

#endif
