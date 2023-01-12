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
}

void ItemSorter::add(Item *itemsBuf, int n) {
    std::unique_lock lock(m_mut);
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

    std::unique_lock lock(m_mut);
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
    std::unique_lock lock(m_mut);
    return m_items.size();
}

void ItemSorter::setQuery(std::string m_query) {
    if (this->m_query.size() < m_query.size()) {
        m_queryDeleted = !m_query.starts_with(this->m_query);
    }
    else {
        m_queryDeleted = this->m_query != m_query;
    }

    if (m_queryDeleted) {
        m_heuristicIdx = 0;
    }

    m_queryChanged = true;
    this->m_query = m_query;
}

void ItemSorter::calcHeuristics() {
    std::unique_lock lock(m_mut);

    if (m_queryChanged && m_heuristicIdx) {
        calcHeuristics(m_query.c_str(), true, 0, m_heuristicIdx);
    }
    int n = m_items.size();
    if (n && n > m_heuristicIdx) {
        calcHeuristics(m_query.c_str(), false, m_heuristicIdx, n);
    }
    m_heuristicIdx = n;

    m_queryChanged = false;
}

void ItemSorter::calcHeuristics(const char* m_query, bool skipEmpty, int start,
        int end)
{
    std::function<void(Item*, int)> f;
    ItemMatcher matcher;
    std::vector<std::string> words = split(m_query, ' ');

    m_isSorted = strlen(m_query) > 0;

    if (m_isSorted) {
        f = [&] (Item *item, int n) {
            for (int i = 0; i < n; i++) {
                if (skipEmpty && item->heuristic == INT_MAX) {
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
    if (idx + n > m_items.size()) {
        n = m_items.size() - idx;
    }

    if (idx + n > m_sortIdx) {
        sort(idx + n);
    }

    std::unique_lock lock(m_mut);

    for (int i = 0; i < n; i++) {
        buffer[i] = m_items[idx + i];
    }

    return n;
}
