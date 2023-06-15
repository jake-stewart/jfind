#ifndef ITEM_SORTER_HPP
#define ITEM_SORTER_HPP

#include "event_dispatch.hpp"
#include "item.hpp"
#include "item_matcher.hpp"
#include "logger.hpp"
#include <fstream>
#include <mutex>
#include <thread>
#include <vector>
#include <string>

class ItemSorter : public EventListener
{
public:
    ItemSorter(std::string startQuery);
    void setMatcher(ItemMatcher *matcher);
    int size() const;
    int copyItems(Item *buffer, int idx, int n);
    void onEvent(std::shared_ptr<Event> event) override;
    void onLoop() override;
    void onStart() override;

private:
    ItemMatcher *m_matcher;

    bool m_sorterThreadActive = false;
    std::thread *m_sorterThread;
    void sorterThread();
    void endSorterThread();

    void sort(int n);
    void setQuery();
    void calcHeuristics(bool queryChanged);
    void calcHeuristics(bool newItems, int start, int end);

    void addNewItems();
    void sortItems();

    EventDispatch &m_dispatch = EventDispatch::instance();
    Logger m_logger = Logger("ItemSorter");

    std::mutex m_sorter_mut;
    std::condition_variable m_sorter_cv;

    std::mutex m_items_mut;

    bool m_hasNewItems = false;
    std::vector<Item> *m_newItems;

    int m_heuristicIdx;
    int m_sortIdx;

    Item m_firstItems[256];
    int m_firstItemsSize = 0;

    std::vector<Item> m_items;
    bool m_isSorted;
    std::string m_query;
    bool m_queryChanged;
    std::string m_newQuery;
};

#endif
