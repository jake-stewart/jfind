#include "../include/file_item_reader.hpp"
#include <cstring>
#include <cstdlib>
#include <chrono>
#include <thread>

extern "C" {
#include <unistd.h>
#include <fcntl.h>
}

using namespace std::chrono_literals;
using std::chrono::time_point;
using std::chrono::system_clock;

FileItemReader::FileItemReader(FILE *file) {
    m_file = file;
    m_interval.setInterval(50ms);
    m_itemReader.setFile(file);
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
    m_logger.log("received %s", getEventNames()[event->getType()]);
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
    m_dispatch.dispatch(std::make_shared<NewItemsEvent>(&m_items.getSecondary()));
    m_items.getPrimary().clear();
}

void FileItemReader::onStart() {
    m_logger.log("started");
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
    for (int i = 0; i < 128; i++) {

        bool success = read();
        if (!success) {
            return false;
        }

        if (system_clock::now() - start > 50ms) {
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
