#ifndef ITEM_READER_HPP
#define ITEM_READER_HPP

#include "cancellable_reader.hpp"
#include "buffered_reader.hpp"
#include "item.hpp"
#include <cstdio>

class ItemReader
{
    BufferedReader m_reader;
    int m_itemId = 0;
    FILE *m_file;

public:
    void setFile(FILE *file);
    BufferedReader &getReader();
    bool read(Item &item);

private:
    bool readWithHints(Item &item);
    bool readWithoutHints(Item &item);
};

#endif
