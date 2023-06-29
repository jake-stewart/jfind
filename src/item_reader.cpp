#include "../include/item_reader.hpp"
#include "../include/config.hpp"

#include <cstring>

void ItemReader::setFd(int fd) {
    m_reader.setFd(fd);
}

BufferedReader &ItemReader::getReader() {
    return m_reader;
}

bool ItemReader::read(Item &item) {
    item.text = m_reader.getlines(Config::instance().showHints ? 2 : 1);
    item.index = m_itemId++;
    return item.text;
}
