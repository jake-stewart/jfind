#include "../include/item_reader.hpp"
#include "../include/config.hpp"

#include <cstring>

void ItemReader::setFile(FILE *file) {
    m_reader.setFile(file);
}

bool ItemReader::read(Item &item) {
    if (Config::instance().showHints) {
        return readWithHints(item);
    }
    return readWithoutHints(item);
}


bool ItemReader::readWithoutHints(Item &item) {
    size_t size;
    char *buf = nullptr;

    if (m_reader.getline(&buf, &size) < 0) {
        free((void*)buf);
        return false;
    }

    buf[strcspn(buf, "\n")] = 0;
    item.text = buf;
    item.index = m_itemId++;

    return true;
}

bool ItemReader::readWithHints(Item &item) {
    size_t size;
    char *buf = nullptr;
    char *secondBuf = nullptr;

    if (m_reader.getline(&buf, &size) < 0) {
        free((void*)buf);
        return false;
    }
    if (m_reader.getline(&secondBuf, &size) < 0) {
        free((void*)buf);
        free((void*)secondBuf);
        return false;
    }

    buf[strcspn(buf, "\n")] = 0;
    secondBuf[strcspn(secondBuf, "\n")] = 0;

    char *thirdBuf = (char*)malloc(strlen(buf) + strlen(secondBuf) + 2);
    strcpy(thirdBuf, buf);
    strcpy(thirdBuf + strlen(buf) + 1, secondBuf);
    free(secondBuf);
    free(buf);

    item.text = thirdBuf;
    item.index = m_itemId++;
    item.heuristic = 0;

    return true;
}

void ItemReader::cancel() {
    m_reader.cancel();
}
