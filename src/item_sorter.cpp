#include "../include/item_sorter.hpp"
#include "../include/util.hpp"
#include "../include/item_matcher.hpp"
#include "../include/thread_manager.hpp"
#include <unordered_map>
#include <cstring>
#include <climits>

using namespace std::chrono_literals;

ItemSorter::ItemSorter() {
    m_isSorted = false;
    m_heuristicIdx = 0;
    m_sortIdx = 0;

    m_dispatch.subscribe(this, QUERY_CHANGE_EVENT);
    m_dispatch.subscribe(this, NEW_ITEMS_EVENT);
    m_dispatch.subscribe(this, QUIT_EVENT);
}

void ItemSorter::add(Item *itemsBuf, int n) {
    LayeredLock lock(m_mut);
    m_items.insert(m_items.end(), itemsBuf, itemsBuf + n);
}

bool sortFunc(Item& l, Item &r) {
    if (l.heuristic == r.heuristic) {
        if (strlen(l.text) == strlen(r.text)) {
            return l.index < r.index;
        }
        return strlen(l.text) < strlen(r.text);
    }
    return l.heuristic > r.heuristic;
}

bool sortEmptyFunc(Item& l, Item &r) {
    return l.index < r.index;
}

void ItemSorter::sort(int n) {
    LayeredLock lock(m_mut);
    if (n <= m_sortIdx) {
        return;
    }
    if (m_sortIdx + n > m_items.size()) {
        n = m_items.size() - m_sortIdx;
    }
    std::function<bool(Item& l, Item &r)> f;
    f = m_isSorted ? sortFunc : sortEmptyFunc;

    std::partial_sort(m_items.begin() + m_sortIdx,
            m_items.begin() + m_sortIdx + n, m_items.end(), f);
    m_sortIdx += n;
}

int ItemSorter::size() {
    LayeredLock lock(m_mut);
    return m_items.size();
}

void ItemSorter::setQuery(std::string query) {
    LayeredLock lock(m_mut);

    bool backspaced;

    if (m_query.size() < query.size()) {
        backspaced = !query.starts_with(m_query);
    }
    else {
        backspaced = m_query != query;
    }

    if (backspaced) {
        m_heuristicIdx = 0;
    }

    m_queryChanged = true;
    m_query = query;
}

void ItemSorter::calcHeuristics(bool queryChanged) {
    LayeredLock lock(m_mut);
    if (queryChanged && m_heuristicIdx) {
        m_logger.log("itemSorter: fast calcHeuristics for %d items", m_heuristicIdx);
        calcHeuristics(m_query.c_str(), false, 0, m_heuristicIdx);
    }
    int n = m_items.size();
    if (n && n > m_heuristicIdx) {
        m_logger.log("itemSorter: slow calcHeuristics for %d items", n - m_heuristicIdx);
        calcHeuristics(m_query.c_str(), true, m_heuristicIdx, n);
    }
    m_heuristicIdx = n;

    m_queryChanged = false;
}

void ItemSorter::calcHeuristics(const char* query, bool newItems, int start, int end)
{
    LayeredLock lock(m_mut);
    std::function<void(Item*, int)> f;
    ItemMatcher matcher;
    std::vector<std::string> words = split(query, ' ');

    m_isSorted = strlen(query) > 0;

    if (m_isSorted) {
        f = [&] (Item *item, int n) {
            for (int i = 0; i < n; i++) {
                if (m_queryChanged) {
                    return;
                }
                if (!newItems && item->heuristic == BAD_HEURISTIC) {
                    item++;
                    continue;
                }
                (*item).heuristic = matcher.calc((*item).text, words);
                item++;
            }
        };
    }
    else {
        f = [] (Item *item, int n) {
            for (int i = 0; i < n; i++) {
                item->heuristic = 0;
                item++;
            }
        };
    }

    ThreadManager<Item> manager(f);
    manager.setNumThreads(4);
    manager.setThreshold(1024);
    manager.run(m_items.data() + start, end - start);

    m_sortIdx = 0;
}

std::vector<Item>& ItemSorter::getItems() {
    return m_items;
}

int ItemSorter::copyItems(Item *buffer, int idx, int n) {
    LayeredLock lock(m_mut);
    if (idx + n > m_items.size()) {
        n = m_items.size() - idx;
    }

    if (idx + n > m_sortIdx) {
        sort(idx + n);
    }

    // std::unique_lock lock(m_mut);

    for (int i = 0; i < n; i++) {
        buffer[i] = m_items[idx + i];
    }

    return n;
}

void ItemSorter::onLoop() {
    if (!m_queryChanged && !m_hasNewItems) {
        return awaitEvent();
    }
    LayeredLock lock(m_mut);
    // std::unique_lock lock(m_mut);
    addNewItems();
    sortItems();
}

void ItemSorter::addNewItems() {
    if (!m_hasNewItems) {
        return;
    }
    m_hasNewItems = false;
    LayeredLock lock(m_mut);
    m_items.insert(m_items.end(), m_newItems->data(),
                   m_newItems->data() + m_newItems->size());
    m_dispatch.dispatch(std::make_shared<ItemsAddedEvent>());
}

void ItemSorter::sortItems() {
    bool queryChanged = m_queryChanged;
    m_queryChanged = false;

    // calc will cancel upon query change
    // this way, jfind can quickly restart calc with new query
    calcHeuristics(queryChanged);

    if (!m_queryChanged) {
        // sort the first few items on the sorter thread. this is to remove the
        // delay on the main thread, which the user could notice
        sort(256);
    }
    m_dispatch.dispatch(std::make_shared<ItemsSortedEvent>());
}

void ItemSorter::onEvent(std::shared_ptr<Event> event) {
    m_logger.log("ItemSorter: received %s", getEventNames()[event->getType()]);
    switch (event->getType()) {
        case QUERY_CHANGE_EVENT: {
            QueryChangeEvent *queryChangeEvent = (QueryChangeEvent*)event.get();
            setQuery(queryChangeEvent->getQuery());
            break;
        }

        case NEW_ITEMS_EVENT: {
            NewItemsEvent *newItemsEvent = (NewItemsEvent*)event.get();
            m_newItems = newItemsEvent->getItems();
            m_hasNewItems = true;
            break;
        }

        default:
            break;
    }
}
