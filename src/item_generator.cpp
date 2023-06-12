#include "../include/item_generator.hpp"
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

ItemGenerator::ItemGenerator(std::string command) {
    m_interval.setInterval(INTERVAL);
    m_dispatch.subscribe(this, QUIT_EVENT);
    m_dispatch.subscribe(this, QUERY_CHANGE_EVENT);
}

bool ItemGenerator::readFirstBatch() {
    time_point start = system_clock::now();
    for (int i = 0; i < 128; i++) {
        if (!readItem()) {
            return false;
        }
        if (system_clock::now() - start > INTERVAL) {
            break;
        }
    }
    return true;
}

void ItemGenerator::onStart() {
    m_logger.log("started");
    startChildProcess();

    m_interval.start();
}

void ItemGenerator::startChildProcess() {
    if (m_process.getState() != ProcessState::None) {
        m_logger.log("cannot start process with one active");
        return;
    }

    std::string command = applyQuery(Config::instance().command, m_query);
    const char* argv[] = {"/bin/sh", "-c", command.c_str(), NULL};
    bool success = m_process.start((char**)argv);
    if (!success) {
        exit(EXIT_FAILURE);
    }

    m_itemReader.setFile(m_process.getStdout());
    if (readFirstBatch()) {
        dispatchItems();
    }
    else {
        endChildProcess();
    }
}

void ItemGenerator::endChildProcess() {
    if (m_process.getState() == ProcessState::None) {
        return;
    }
    if (!m_process.end()) {
        exit(EXIT_FAILURE);
    }
}

bool ItemGenerator::readItem() {
    Item item;
    bool success = m_itemReader.read(item);
    if (!success) {
        return false;
    }
    std::unique_lock lock(m_mut);
    m_items.getSecondary().push_back(item);
    return true;
}

void ItemGenerator::dispatchItems() {
    m_dispatch.dispatch(std::make_shared<ItemsSortedEvent>(m_query));
}

void ItemGenerator::onLoop() {
    if (m_interval.ticked()) {
        m_interval.restart();

        if (m_queryChanged) {
            {
                std::unique_lock lock(m_mut);
                m_queryChanged = false;
                m_query = m_newQuery;
                m_items.swap();
                for (const Item &item : m_items.getPrimary()) {
                    free((void*)item.text);
                }
                m_items.getPrimary().clear();
            }
            endChildProcess();
            startChildProcess();
            m_dispatch.dispatch(std::make_shared<AllItemsReadEvent>(false));
        }

        dispatchItems();
    }
    if (readItem()) {
        if (m_items.getPrimary().size() >= 4096) {
            dispatchItems();
            m_dispatch.dispatch(std::make_shared<AllItemsReadEvent>(true));
            if (m_queryChanged) {
                return;
            }
            endChildProcess();
            awaitEvent();
        }
    }
    else {
        endChildProcess();
        dispatchItems();
        m_dispatch.dispatch(std::make_shared<AllItemsReadEvent>(true));
        if (!m_queryChanged) {
            awaitEvent();
        }
    }
}

void ItemGenerator::onEvent(std::shared_ptr<Event> event) {
    m_logger.log("received %s", getEventNames()[event->getType()]);

    switch (event->getType()) {
        case QUIT_EVENT:
            endChildProcess();
            m_interval.end();
            break;
        case QUERY_CHANGE_EVENT: {
            std::unique_lock lock(m_mut);
            QueryChangeEvent *queryChangeEvent
                = (QueryChangeEvent *)event.get();
            m_newQuery = queryChangeEvent->getQuery();
            m_queryChanged = true;
            break;
        }
        default:
            break;
    }
}

int ItemGenerator::copyItems(Item *buffer, int idx, int n) {
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

int ItemGenerator::size() {
    std::unique_lock lock(m_mut);
    return m_items.getPrimary().size();
}
