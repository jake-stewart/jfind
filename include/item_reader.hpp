#ifndef ITEM_READER_HPP
#define ITEM_READER_HPP

#include <vector>
#include <stdio.h>
#include "item.hpp"

class ItemReader {
    public:
        ItemReader();

        void setFile(FILE *file);
        void setReadHints(bool readHints);
        bool read(std::vector<Item>& itemsBuf);

    private:
        int m_itemId;
        bool m_readHints;
        FILE *m_file;

        bool readWithHints(std::vector<Item>& ItemsBuf);
        bool readWithoutHints(std::vector<Item>& ItemsBuf);
};

#endif
