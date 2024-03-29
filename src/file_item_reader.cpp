#include "../include/file_item_reader.hpp"
#include "../include/logger.hpp"
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <thread>

extern "C" {
#include <fcntl.h>
#include <unistd.h>
}

using namespace std::chrono_literals;
using std::chrono::system_clock;
using std::chrono::time_point;

FileItemReader::FileItemReader(int fd) {
    m_interval.setInterval(10ms);
    m_itemReader.setFd(fd);
    m_dispatch.subscribe(this, ITEMS_ADDED_EVENT);
}

bool FileItemReader::read() {
    Item item;
    if (!m_itemReader.read(item)) {
        return false;
    }
    m_items.getPrimary().push_back(item);
    return true;
}

void FileItemReader::onEvent(std::shared_ptr<Event> event) {
    LOG("received %s", getEventNames()[event->getType()]);
    switch (event->getType()) {
        case ITEMS_ADDED_EVENT:
            m_itemsRead = true;
            break;
        default:
            break;
    }
}

void FileItemReader::dispatchItems() {
    if (!m_items.getPrimary().size()) {
        return;
    }
    m_itemsRead = false;
    m_items.swap();
    m_dispatch.dispatch(std::make_shared<NewItemsEvent>(&m_items.getSecondary())
    );
    m_items.getPrimary().clear();
}

void FileItemReader::onStart() {
    LOG("started");
    m_itemsRead = true;

    bool success = readFirstBatch();
    dispatchItems();
    if (success) {
        m_interval.start();
    }
    else {
        m_dispatch.dispatch(std::make_shared<AllItemsReadEvent>(true));
        end();
    }
}

bool FileItemReader::readFirstBatch() {
    time_point start = system_clock::now();
    for (int i = 0; i < 1; i++) {
        bool success = read();
        if (!success) {
            return false;
        }
        if (system_clock::now() - start > 10ms) {
            break;
        }
    }
    return true;
}

void FileItemReader::onLoop() {
    if (m_itemsRead && m_interval.ticked()) {
        m_interval.restart();
        dispatchItems();
    }
    if (!read()) {
        if (m_items.getPrimary().size()) {
            if (!m_itemsRead) {
                awaitEvent();
            }
            dispatchItems();
        }
        m_interval.end();
        m_dispatch.dispatch(std::make_shared<AllItemsReadEvent>(true));
        end();
    }
}
