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
    std::string quoted = "'" + std::regex_replace(query, QUOTE_REGEX, "'\"'\"'") + "'";
    if (!std::regex_search(str, PLACEHOLDER_REGEX)) {
        return str + " " + quoted;
    }
    return std::regex_replace(str, PLACEHOLDER_REGEX, quoted);
}

ItemGenerator::ItemGenerator(std::string command) {
    m_dispatch.subscribe(this, QUIT_EVENT);
    m_dispatch.subscribe(this, QUERY_CHANGE_EVENT);
}

void ItemGenerator::readFirstBatch() {
    time_point start = system_clock::now();
    for (int i = 0; i < 128; i++) {
        if (!readItem()) {
            break;
        }
        if (system_clock::now() - start > INTERVAL) {
            break;
        }
    }
}

void ItemGenerator::onStart() {
    startChildProcess();

    m_intervalActive = true;
    m_intervalThread = new std::thread(
            &ItemGenerator::intervalThread, this);
}

void ItemGenerator::intervalThread() {
    m_intervalPassed = false;
    while (m_intervalActive) {
        std::unique_lock lock(m_intervalMut);
        m_intervalCv.wait_for(lock, INTERVAL);
        m_intervalPassed = true;
    }
}

void ItemGenerator::startChildProcess() {
    if (m_processActive) {
        m_logger.log("already active");
        exit(1);
    }
    m_processActive = true;

    if (pipe(m_pipefd) == -1) {
        m_logger.log("pipe failed");
        exit(EXIT_FAILURE);
    }

    m_child_pid = fork();

    if (m_child_pid < 0) {
        m_logger.log("forking failed");
        exit(EXIT_FAILURE);
    }

    if (m_child_pid == 0) {
        close(m_pipefd[READ]);
        if (dup2(m_pipefd[WRITE], STDOUT_FILENO) == -1) {
            _exit(EXIT_FAILURE);
        }
        close(STDERR_FILENO);
        std::string command = applyQuery(Config::instance().command, m_query);
        execlp("/bin/sh", "sh", "-c", command.c_str(), NULL);
        _exit(EXIT_FAILURE);
    }

    close(m_pipefd[WRITE]);
    m_file = fdopen(m_pipefd[READ], "r");
    if (m_file == NULL) {
        m_logger.log("fdopen failed");
        exit(EXIT_FAILURE);
    }

    readFirstBatch();
    dispatchItems();
}

void ItemGenerator::endChildProcess() {
    if (!m_processActive) {
        return;
    }
    m_processActive = false;
    kill(m_child_pid, SIGTERM);
    close(m_pipefd[READ]);
    int status;
    waitpid(m_child_pid, &status, 0);
}

bool ItemGenerator::readItem() {
    char *buf = nullptr;
    size_t size = 0;

    if (getline(&buf, &size, m_file) < 0) {
        return false;
    }
    buf[strcspn(buf, "\n")] = 0;

    Item item;
    item.text = buf;
    item.index = m_itemId++;

    std::unique_lock lock(m_mut);
    m_items.push_back(item);

    return true;
}

void ItemGenerator::dispatchItems() {
    m_dispatch.dispatch(std::make_shared<ItemsSortedEvent>(m_query));
}

void ItemGenerator::endInterval() {
    if (!m_intervalActive) {
        return;
    }
    {
        std::unique_lock lock(m_intervalMut);
        m_intervalActive = false;
    }
    m_intervalCv.notify_one();
    m_intervalThread->join();
    delete m_intervalThread;
}

void ItemGenerator::onLoop() {
    if (m_intervalPassed) {
        m_intervalPassed = false;

        if (m_queryChanged) {
            m_logger.log("query changed");
            m_queryChanged = false;
            endChildProcess();
            {
                std::unique_lock lock(m_mut);
                m_items.clear();
            }
            startChildProcess();
        }

        dispatchItems();
    }
    if (readItem()) {
        bool reachedItemLimit;
        {
            std::unique_lock lock(m_mut);
            reachedItemLimit = m_items.size() > 2048;
        }
        if (reachedItemLimit) {
            endChildProcess();
            awaitEvent();
        }
    }
    else {
        dispatchItems();
        m_dispatch.dispatch(std::make_shared<AllItemsReadEvent>(true));
        awaitEvent();
    }
}

void ItemGenerator::onEvent(std::shared_ptr<Event> event) {
    m_logger.log("received %s", getEventNames()[event->getType()]);

    switch (event->getType()) {
        case QUIT_EVENT:
            endChildProcess();
            endInterval();
            break;
        case QUERY_CHANGE_EVENT: {

            QueryChangeEvent *queryChangeEvent
                = (QueryChangeEvent *)event.get();
            m_query = queryChangeEvent->getQuery();
            m_queryChanged = true;
            break;
        }
        default:
            break;
    }
}

int ItemGenerator::copyItems(Item *buffer, int idx, int n) {
    std::unique_lock lock(m_mut);
    if (idx + n > m_items.size()) {
        n = m_items.size() - idx;
    }
    for (int i = 0; i < n; i++) {
        buffer[i] = m_items[idx + i];
    }
    return n;
}

int ItemGenerator::size() {
    std::unique_lock lock(m_mut);
    return m_items.size();
}
