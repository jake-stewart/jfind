#include "../include/process_item_reader.hpp"
#include "../include/config.hpp"
#include "../include/buffered_reader.hpp"
#include "../include/logger.hpp"
#include "../include/util.hpp"
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <regex>

extern "C" {
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
}

using namespace std::chrono_literals;
using std::chrono::system_clock;
using std::chrono::time_point;

#define INTERVAL 50ms
#define READ_BATCH 128

#define READ 0
#define WRITE 1

ProcessItemReader::ProcessItemReader(
    std::string command, std::string startQuery
) {
    m_query = startQuery;
    m_interval.setInterval(INTERVAL);
    m_dispatch.subscribe(this, QUERY_CHANGE_EVENT);
    m_dispatch.subscribe(this, ITEMS_REQUEST_EVENT);
}

bool ProcessItemReader::readFirstBatch() {
    time_point start = system_clock::now();
    bool success;
    for (int i = 0; i < READ_BATCH; i++) {
        Item item;
        success = m_itemReader.read(item);
        if (!success) {
            break;
        }
        m_items.getSecondary().push_back(item);
        if (system_clock::now() - start > INTERVAL) {
            break;
        }
    }
    m_items.swap();
    return success;
}

bool ProcessItemReader::readItem() {
    Item item;
    bool success = m_itemReader.read(item);
    if (!success) {
        return false;
    }

    std::unique_lock lock(m_mut);
    m_items.getPrimary().push_back(item);
    return true;
}

void ProcessItemReader::onStart() {
    LOG("started");
    m_interval.start();

    if (m_query.size()) {
        startChildProcess();
    }
    else {
        dispatchAllRead(true);
    }
}

void ProcessItemReader::startChildProcess() {
    std::string command = applyQuery(Config::instance().command, m_query);
    LOG("command = %s", command.c_str());
    const char *argv[] = {"/bin/sh", "-c", command.c_str(), NULL};
    if (!m_process.start((char **)argv)) {
        LOG("failed to start process errno=%d", errno);
        exit(EXIT_FAILURE);
    }
    m_readMax = READ_BATCH;
    LOG("setting fd to %d", m_process.getFd());
    m_itemReader.setFd(m_process.getFd());
    bool success = readFirstBatch();
    dispatchItems();
    dispatchAllRead(!success);
    if (!success) {
        LOG("reading first batch failed %d", m_items.getPrimary().size());
        m_process.end();
        awaitEvent();
    }
}

void ProcessItemReader::dispatchItems() {
    m_dispatch.dispatch(std::make_shared<ItemsSortedEvent>(m_query));
}

void ProcessItemReader::dispatchAllRead(bool value) {
    m_dispatch.dispatch(std::make_shared<AllItemsReadEvent>(value));
}

void ProcessItemReader::onLoop() {
    if (m_queryChanged) {
        if (!m_interval.ticked()) {
            awaitEvent(m_interval.getRemaining());
            return;
        }
        m_interval.restart();
        m_queryChanged = false;
        m_query = m_newQuery;

        for (char *buffer : m_previousReaderBuffers) {
            free(buffer);
        }
        m_previousReaderBuffers = m_itemReader.getReader().getBuffers();
        m_itemReader.getReader().reset();
        m_items.getSecondary().clear();

        if (m_query.size()) {
            startChildProcess();
        }
        else {
            m_items.swap();
            dispatchItems();
            dispatchAllRead(true);
        }
        return;
    }

    if (m_process.getState() != ProcessState::Active) {
        return awaitEvent();
    }

    bool success = readItem();
    if (success) {
        if (m_items.getPrimary().size() >= m_readMax) {
            dispatchItems();
            dispatchAllRead(true);
            m_process.suspend();
        }
        else if (m_interval.ticked()) {
            m_interval.restart();
            dispatchItems();
        }
    }
    else {
        m_process.end();
        dispatchItems();
        dispatchAllRead(true);
        if (!m_queryChanged) {
            return awaitEvent();
        }
    }
}

void ProcessItemReader::onEvent(std::shared_ptr<Event> event) {
    LOG("received %s", getEventNames()[event->getType()]);

    switch (event->getType()) {
        case QUERY_CHANGE_EVENT: {
            QueryChangeEvent *queryChangeEvent = (QueryChangeEvent *)event.get(
            );
            m_newQuery = queryChangeEvent->getQuery();
            m_queryChanged = true;
            m_process.end();
            break;
        }
        case ITEMS_REQUEST_EVENT: {
            std::unique_lock lock(m_mut);
            if (m_items.getPrimary().size() >= m_readMax) {
                m_readMax += READ_BATCH;
                m_process.resume();
            }
            break;
        }
        default:
            break;
    }
}

int ProcessItemReader::copyItems(Item *buffer, int idx, int n) {
    std::unique_lock lock(m_mut);
    std::vector<Item> &items = m_items.getPrimary();
    if (idx + n > items.size()) {
        n = items.size() - idx;
    }
    for (int i = 0; i < n; i++) {
        buffer[i] = items[idx + i];
    }
    return n;
}

int ProcessItemReader::size() {
    std::unique_lock lock(m_mut);
    return m_items.getPrimary().size();
}

const std::vector<Item> &ProcessItemReader::getItems() {
    switch (m_process.getState()) {
        case ProcessState::Suspended:
            m_process.resume();
        case ProcessState::Active:
            m_readMax = UINT_MAX;
            while (readItem());
        default:
            return m_items.getPrimary();
    }
}
