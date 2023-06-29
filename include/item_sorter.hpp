#ifndef ITEM_SORTER_HPP
#define ITEM_SORTER_HPP

#include "event_dispatch.hpp"
#include "item.hpp"
#include "item_matcher.hpp"
#include <fstream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

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

    std::mutex m_sorterMut;
    std::condition_variable m_sorterCv;

    std::mutex m_itemsMut;
    std::mutex m_firstItemsMut;

    bool m_hasNewItems = false;
    std::vector<Item> *m_newItems;

    int m_heuristicIdx;
    int m_sortIdx;

    Item m_firstItems[256];
    int m_firstItemsSize = 0;

private:
    std::vector<Item> m_items;
    bool m_isSorted;

    bool m_queryChanged;
    std::string m_query;
    std::string m_newQuery;
};

#endif
