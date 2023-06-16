#ifndef ITEM_READER_HPP
#define ITEM_READER_HPP

#include "cancellable_reader.hpp"
#include "item.hpp"
#include <cstdio>

class ItemReader
{
    int m_itemId = 0;
    FILE *m_file;

public:
    void setFile(FILE *file);
    bool read(Item &item);

private:
    bool readWithHints(Item &item);
    bool readWithoutHints(Item &item);
};

#endif
