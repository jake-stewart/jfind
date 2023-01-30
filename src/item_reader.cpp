#include "../include/item_reader.hpp"

ItemReader::ItemReader() {
    m_file = stdin;
    m_readHints = false;
    m_itemId = 0;
}

void ItemReader::setFile(FILE *fp) {
    m_file = fp;
}

void ItemReader::setReadHints(bool readHints) {
    m_readHints = readHints;
}

bool ItemReader::read(std::vector<Item>& itemsBuf) {
    if (m_readHints) {
        return readWithHints(itemsBuf);
    }
    return readWithoutHints(itemsBuf);
}


bool ItemReader::readWithHints(std::vector<Item>& itemsBuf) {
    size_t size;
    char *buf = nullptr;
    char *secondBuf = nullptr;

    if (getline(&buf, &size, m_file) < 0) {
        return false;
    }

    if (getline(&secondBuf, &size, m_file) < 0) {
        return false;
    }

    buf[strcspn(buf, "\n")] = 0;
    secondBuf[strcspn(secondBuf, "\n")] = 0;

    char *thirdBuf = (char*)malloc(strlen(buf) + strlen(secondBuf) + 2);

    strcpy(thirdBuf, buf);
    strcpy(thirdBuf + strlen(buf) + 1, secondBuf);

    free(secondBuf);
    free(buf);

    Item item;
    item.text = thirdBuf;
    item.index = m_itemId++;
    item.heuristic = 0;
    itemsBuf.push_back(item);

    return true;
}

bool ItemReader::readWithoutHints(std::vector<Item>& itemsBuf) {
    size_t size;
    char *buf = nullptr;

    if (getline(&buf, &size, m_file) < 0) {
        return false;
    }

    buf[strcspn(buf, "\n")] = 0;

    Item item;
    item.text = buf;
    item.index = m_itemId++;
    itemsBuf.push_back(item);

    buf = nullptr;

    return true;
}
