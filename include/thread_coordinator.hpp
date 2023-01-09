#ifndef THREAD_COORDINATOR_HPP
#define THREAD_COORDINATOR_HPP

#include "item.hpp"
#include "history_manager.hpp"
#include "user_interface.hpp"
#include "item_reader.hpp"

#include <mutex>
#include <thread>
#include <condition_variable>

enum ThreadState {
    INACTIVE,
    ACTIVE,
    CLOSING
};

template <class T>
class DoubleBuffer {
    public:
        void swap() {
            m_isSwapped = !m_isSwapped;
        }

        T& getPrimary() {
            return m_isSwapped ? m_first : m_second;
        }

        T& getSecondary() {
            return m_isSwapped ? m_second : m_first;
        }

    private:
        bool m_isSwapped = false;
        T m_first;
        T m_second;
};

class ThreadCoordinator {
    public:
        void setHistoryManager(HistoryManager *historyManager);
        void setUserInterface(UserInterface *userInterface);
        void setItemSorter(ItemSorter *sorter);
        void setItemReader(ItemReader itemReader);
        void start();

    private:
        // reader thread reads into primary buffer. sorter thread copies from
        // secondary buffer into items. buffer is swapped by reader thread once
        // sorter is done
        DoubleBuffer<std::vector<Item>> m_itemsBuf;

        // userInputBlocked flagged true when the main thread is waiting for a
        // user to press a key. when this is the case, the sorter thread is
        // safe to draw the screen. if the flag is false, then the sorter
        // thread should instead set requiresRedraw to true to indicate to the
        // main thread that a redraw is required once finished handling user
        // input
        bool m_userInputBlocked = false;
        bool m_requiresRedraw = false;

        // the sorter thread will copy items from the secondary buffer into the
        // main items pool when this flag is true. once finished, this flag is
        // set to false and the reader thread can swap buffers
        bool m_secondaryBufHasItems = false;

        // once the main thread has finished handling user input, it will check
        // whether the query has changed. when the query has changed, the
        // sorter thread will provide the query to the main items pool and
        // reset the flag
        bool m_queryChanged = false;

        // the sorter reader cv and mutex are used for coordinating the first
        // and final batch reads. the first batch read is performed quickly so
        // that there is no flicker when first launching jfind
        std::condition_variable m_sorterReaderCv;
        std::mutex m_sorterReaderMut;

        // the sorter main cv tells the sorter thread when the query has
        // changed it also fires when the sorter thread should terminate. the
        // sorter main mutex is used to share the user interface between the
        // sorter thread and main thread
        std::condition_variable m_sorterMainCv;
        std::mutex m_sorterMainMutex;

        std::thread m_sorterThread;
        std::thread m_readerThread;
        ThreadState m_sorterThreadState = INACTIVE;
        ThreadState m_readerThreadState = INACTIVE;

        ItemReader m_itemReader;
        ItemSorter *m_sorter;
        HistoryManager *m_historyManager;
        UserInterface *m_userInterface;

        // reader thread funcs
        void readerThreadFunc();
        void readFirstBatch();
        int readItem();

        // sorter thread funcs
        void sorterThreadFunc();
        void waitForFirstBatch();
        void addItems();
        void sortItems();

        // main thread funcs
        void handleUserInput();
        void mainThreadFunc();

        void startThreads();
        void endThreads();
};


#endif
