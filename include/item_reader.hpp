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

public:
    void setFd(int fd);
    BufferedReader &getReader();
    bool read(Item &item);
};

#endif
