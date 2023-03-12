#include "../include/item_reader.hpp"
#include <cstring>
#include <cstdlib>
#include <chrono>
#include <thread>

namespace chrono = std::chrono;
using namespace std::chrono_literals;

#define INTERVAL 50ms

ItemReader::ItemReader() {
    m_file = stdin;
    m_readHints = false;
    m_itemId = 0;

    m_dispatch.subscribe(this, QUIT_EVENT);
    m_dispatch.subscribe(this, ITEMS_ADDED_EVENT);
}

void ItemReader::setFile(FILE *fp) {
    m_file = fp;
}

void ItemReader::setReadHints(bool readHints) {
    m_readHints = readHints;
}

bool ItemReader::read() {
    if (m_readHints) {
        return readWithHints();
    }
    return readWithoutHints();
}

bool ItemReader::readWithHints() {
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
    m_itemsBuf.getPrimary().push_back(item);

    return true;
}

bool ItemReader::readWithoutHints() {
    size_t size;
    char *buf = nullptr;

    if (getline(&buf, &size, m_file) < 0) {
        return false;
    }

    buf[strcspn(buf, "\n")] = 0;

    Item item;
    item.text = buf;
    item.index = m_itemId++;
    m_itemsBuf.getPrimary().push_back(item);

    buf = nullptr;

    return true;
}

void ItemReader::readFirstBatch() {
    chrono::time_point start = chrono::system_clock::now();
    for (int i = 0; i < 128; i++) {

        bool success = read();
        if (!success) {
            break;
        }

        if (chrono::system_clock::now() - start > INTERVAL) {
            break;
        }
    }
}

void ItemReader::endInterval() {
    if (m_intervalActive) {
        return;
    }
    m_intervalActive = false;
    m_intervalCv.notify_one();
    m_intervalThread->join();
    delete m_intervalThread;
}

void ItemReader::onEvent(std::shared_ptr<Event> event) {
    m_logger.log("ItemReader: received %s", getEventNames()[event->getType()]);
    switch (event->getType()) {
        case QUIT_EVENT: {
            endInterval();
            break;
        }
        case ITEMS_ADDED_EVENT:
            m_itemsRead = true;
            break;
        default:
            break;
    }
}

void ItemReader::dispatchItems() {
    if (!m_itemsBuf.getPrimary().size()) {
        return;
    }
    m_itemsRead = false;
    m_itemsBuf.swap();
    m_dispatch.dispatch(std::make_shared<NewItemsEvent>(&m_itemsBuf.getSecondary()));
    m_itemsBuf.getPrimary().clear();
}

void ItemReader::intervalThread() {
    m_intervalPassed = false;
    while (m_intervalActive) {
        std::unique_lock lock(m_intervalMut);
        m_intervalCv.wait_for(lock, INTERVAL);
        m_intervalPassed = true;
    }
}

void ItemReader::onStart() {
    m_itemsRead = true;

    readFirstBatch();
    dispatchItems();

    m_intervalActive = true;
    m_intervalThread = new std::thread(
            &ItemReader::intervalThread, this);
}

void ItemReader::onLoop() {
    if (m_itemsRead && m_intervalPassed) {
        m_intervalPassed = false;
        dispatchItems();
    }
    if (!read()) {
        if (m_itemsRead || !m_itemsBuf.getPrimary().size()) {
            dispatchItems();
            m_dispatch.dispatch(std::make_shared<AllItemsReadEvent>());
            endInterval();
            end();
        }
        awaitEvent();
    }
}
