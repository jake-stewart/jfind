#include "../include/process_item_reader.hpp"
#include "../include/config.hpp"
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <regex>

extern "C" {
#include <termios.h>
#include <unistd.h>
#include <sys/wait.h>
}

using namespace std::chrono_literals;
using std::chrono::time_point;
using std::chrono::system_clock;

#define INTERVAL 50ms
#define READ_BATCH 128

#define READ 0
#define WRITE 1

std::string applyQuery(const std::string str, const std::string query) {
    static const std::regex PLACEHOLDER_REGEX("\\{\\}");
    static const std::regex QUOTE_REGEX("'");
    std::string quoted = "'" +
        std::regex_replace(query, QUOTE_REGEX, "'\"'\"'") + "'";
    if (!std::regex_search(str, PLACEHOLDER_REGEX)) {
        return str + " " + quoted;
    }
    return std::regex_replace(str, PLACEHOLDER_REGEX, quoted);
}

ProcessItemReader::~ProcessItemReader() {
    freeItems(m_items.getPrimary());
    freeItems(m_items.getSecondary());
}

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
    m_logger.log("started");
    m_interval.start();
    startChildProcess();
}

void ProcessItemReader::startChildProcess() {
    std::string command = applyQuery(Config::instance().command, m_query);
    const char* argv[] = {"/bin/sh", "-c", command.c_str(), NULL};
    if (!m_process.start((char**)argv)) {
        exit(EXIT_FAILURE);
    }
    m_readMax = READ_BATCH;
    m_itemReader.setFile(m_process.getStdout());
    bool success = readFirstBatch();
    dispatchItems();
    dispatchAllRead(!success);
    if (!success) {
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

void ProcessItemReader::freeItems(std::vector<Item> &items) {
    for (const Item &item : items) {
        free((void*)item.text);
    }
    items.clear();
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

        freeItems(m_items.getSecondary());
        startChildProcess();
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
    m_logger.log("received %s", getEventNames()[event->getType()]);

    switch (event->getType()) {
        case QUERY_CHANGE_EVENT: {
            QueryChangeEvent *queryChangeEvent
                = (QueryChangeEvent *)event.get();
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
