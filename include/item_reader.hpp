#ifndef ITEM_READER_HPP
#define ITEM_READER_HPP

#include <cstdio>
#include "item.hpp"
#include "cancellable_reader.hpp"

class ItemReader
{
    int m_itemId = 0;
    FILE *m_file;

public:
    void setFile(FILE *file);
    bool read(Item& item);
    void cancel();

private:
    bool readWithHints(Item &item);
    bool readWithoutHints(Item &item);
};

#endif
