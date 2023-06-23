#include "../include/item_reader.hpp"
#include "../include/config.hpp"

#include <cstring>

void ItemReader::setFile(FILE *file) {
    m_file = file;
}

BufferedReader &ItemReader::getReader() {
    return m_reader;
}

bool ItemReader::read(Item &item) {
    if (Config::instance().showHints) {
        return readWithHints(item);
    }
    return readWithoutHints(item);
}

bool ItemReader::readWithoutHints(Item &item) {
    item.text = m_reader.getline();
    item.index = m_itemId++;
    return item.text;
}

bool ItemReader::readWithHints(Item &item) {
    item.text = m_reader.getline();
    if (!item.text) {
        return false;
    }
    item.index = m_itemId++;
    return m_reader.getline();
}
