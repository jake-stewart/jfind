#ifndef ITEM_SORTER_HPP
#define ITEM_SORTER_HPP

#include "item.hpp"
#include "event_dispatch.hpp"
#include "logger.hpp"
#include <vector>
#include <fstream>
#include <mutex>

class LayeredMutex {
    std::mutex m_mut;
    int m_layer;

public:
    LayeredMutex() {
        m_layer = 0;
    }

    void lock() {
        if (m_layer == 0) {
            m_mut.lock();
        }
        m_layer++;
    }

    void unlock() {
        m_layer--;
        if (m_layer == 0) {
            m_mut.unlock();
        }
    }
};

class LayeredLock {
    LayeredMutex *m_mut;

public:
    LayeredLock(LayeredMutex& mut) {
        m_mut = &mut;
        m_mut->lock();
    }

    ~LayeredLock() {
        m_mut->unlock();
    }
};

class ItemSorter : public EventListener {
public:
    ItemSorter();
    void add(Item *items, int n);
    int size();
    void sort(int n);
    std::vector<Item>& getItems();
    int copyItems(Item *buffer, int idx, int n);
    void onEvent(std::shared_ptr<Event> event);
    void onLoop();

private:
    void setQuery(std::string query);
    void calcHeuristics(bool queryChanged);
    void calcHeuristics(const char *query, bool newItems,
                        int start, int end);

    void addNewItems();
    void sortItems();

    EventDispatch& m_dispatch = EventDispatch::instance();
    Logger& m_logger = Logger::instance();

    LayeredMutex m_mut;

    bool m_hasNewItems = false;
    std::vector<Item> *m_newItems;

    int m_heuristicIdx;
    int m_lastSize;
    int m_sortIdx;

    std::vector<Item> m_items;
    bool m_wakeUp;
    bool m_isSorted;
    std::string m_query;
    bool m_queryChanged;
};

#endif
