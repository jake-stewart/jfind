#include "../include/thread_coordinator.hpp"
#include "../include/ansi_wrapper.hpp"

#include <thread>
#include <unistd.h>
#include <chrono>

using namespace std::chrono_literals;

#define INTERVAL 50ms

int ThreadCoordinator::readItem() {
    int size = m_itemsBuf.getPrimary().size();
    if (!m_itemReader.read(m_itemsBuf.getPrimary())) {
        return false;
    }
    if (m_historyManager) {
        m_historyManager->applyHistory(m_itemsBuf.getPrimary().data() + size);
    }
    return true;
}

void ThreadCoordinator::readFirstBatch() {
    chrono::time_point start = chrono::system_clock::now();
    for (int i = 0; i < 128; i++) {

        bool success = readItem();
        if (!success) {
            break;
        }

        if (chrono::system_clock::now() - start > INTERVAL) {
            break;
        }
    }
}

void ThreadCoordinator::readerThreadFunc() {
    if (isatty(STDIN_FILENO)) {
        std::unique_lock lock(m_sorterReaderMut);
        m_readerThreadState = INACTIVE;
        m_sorterReaderCv.notify_one();
        return;
    }

    readFirstBatch();

    {
        std::unique_lock lock(m_sorterReaderMut);
        if (m_itemsBuf.getPrimary().size()) {
            m_itemsBuf.swap();
            m_itemsBuf.getPrimary().clear();
            m_secondaryBufHasItems = true;
        }
        m_sorterReaderCv.notify_one();
    }

    while (m_readerThreadState == ACTIVE) {
        bool success = readItem();
        if (!m_secondaryBufHasItems) {
            m_itemsBuf.swap();
            m_itemsBuf.getPrimary().clear();
            m_secondaryBufHasItems = true;
        }

        if (!success && m_readerThreadState == ACTIVE) {
            std::unique_lock lock(m_sorterReaderMut);
            while (m_secondaryBufHasItems) {
                m_sorterReaderCv.wait(lock);
            }
            if (m_itemsBuf.getPrimary().size()) {
                m_itemsBuf.swap();
                m_secondaryBufHasItems = true;
                while (m_secondaryBufHasItems) {
                    m_sorterReaderCv.wait(lock);
                }
            }
            break;
        }
    }

    m_readerThreadState = INACTIVE;
}

void ThreadCoordinator::addItems() {
    if (m_secondaryBufHasItems) {
        std::unique_lock lock(m_sorterReaderMut);
        m_sorter->add(m_itemsBuf.getSecondary().data(),
                m_itemsBuf.getSecondary().size());
        m_secondaryBufHasItems = false;
        m_sorterReaderCv.notify_one();
    }
}

void ThreadCoordinator::sortItems() {
    {
        std::unique_lock lock(m_sorterMainMutex);
        m_sorter->setQuery(m_userInterface->getEditor()->getText());
        m_queryChanged = false;
    }

    // calc will cancel upon query change
    // this way, jfind can quickly restart calc with new query
    m_sorter->calcHeuristics(&m_queryChanged);

    if (!m_queryChanged) {
        // sort the first few items on the sorter thread. this is to remove the
        // delay on the main thread, which the user could notice
        m_sorter->sort(256);

        std::unique_lock lock(m_sorterMainMutex);
        if (m_userInputBlocked) {
            m_userInterface->redraw();
            m_requiresRedraw = false;
        }
        else {
            m_requiresRedraw = true;
        }

    }
}

void ThreadCoordinator::waitForFirstBatch() {
    std::unique_lock lock(m_sorterReaderMut);
    if (m_readerThreadState == ACTIVE && !m_secondaryBufHasItems) {
        m_sorterReaderCv.wait(lock);
    }
}

void ThreadCoordinator::sorterThreadFunc() {
    waitForFirstBatch();
    addItems();
    sortItems();

    while (m_sorterThreadState == ACTIVE) {
        if (m_readerThreadState != INACTIVE) {
            std::this_thread::sleep_for(INTERVAL);
            addItems();
        }
        else {
            std::unique_lock lock(m_sorterMainMutex);
            while (!m_queryChanged) {
                if (m_sorterThreadState != ACTIVE) {
                    break;
                }
                m_sorterMainCv.wait(lock);
            }
        }

        if (m_sorterThreadState != ACTIVE) {
            break;
        }
        sortItems();
    }

    m_sorterThreadState = INACTIVE;
}

void ThreadCoordinator::setHistoryManager(HistoryManager *historyManager) {
    m_historyManager = historyManager;
}

void ThreadCoordinator::setUserInterface(UserInterface *userInterface) {
    m_userInterface = userInterface;
}

void ThreadCoordinator::setItemSorter(ItemSorter *sorter) {
    m_sorter = sorter;
}

void ThreadCoordinator::setItemReader(ItemReader itemReader) {
    m_itemReader = itemReader;
}

void ThreadCoordinator::startThreads() {
    m_readerThreadState = ACTIVE;
    m_sorterThreadState = ACTIVE;
    m_sorterThread = std::thread(&ThreadCoordinator::sorterThreadFunc, this);
    m_readerThread = std::thread(&ThreadCoordinator::readerThreadFunc, this);
}

void ThreadCoordinator::handleUserInput() {
    InputReader *reader = m_userInterface->getInputReader();

    m_userInputBlocked = true;
    Key key;
    reader->getKey(&key);

    {
        std::unique_lock lock(m_sorterMainMutex);
        m_userInputBlocked = false;
    }

    while (true) {
        m_userInterface->handleInput(key);
        if (!reader->hasKey()) {
            break;
        }
        reader->getKey(&key);
    }
}

void ThreadCoordinator::mainThreadFunc() {
    Utf8LineEditor *editor = m_userInterface->getEditor();

    while (m_userInterface->isActive()) {
        std::string query = editor->getText();
        handleUserInput();

        if (!m_userInterface->isActive()) {
            break;
        }

        std::unique_lock lock(m_sorterMainMutex);
        if (query != editor->getText()) {
            m_queryChanged = true;
        }
        if (editor->requiresRedraw()) {
            m_userInterface->drawQuery();
        }
        if (m_requiresRedraw) {
            m_userInterface->redraw();
            m_requiresRedraw = false;
        }
        else {
            m_userInterface->focusEditor();
        }
        if (m_queryChanged) {
            m_sorterMainCv.notify_one();
        }
    }
}

void ThreadCoordinator::start() {
    startThreads();
    mainThreadFunc();
    endThreads();
}

void ThreadCoordinator::endThreads() {
    {
        std::unique_lock lock(m_sorterMainMutex);
        m_sorterThreadState = CLOSING;
        m_readerThreadState = CLOSING;

        closeStdin();

        m_sorterMainCv.notify_one();
    }
    m_readerThread.join();
    m_sorterThread.join();
}
