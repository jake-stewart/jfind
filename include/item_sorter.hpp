#ifndef ITEM_SORTER_HPP
#define ITEM_SORTER_HPP

#include "item.hpp"
#include <vector>
#include <fstream>

class ItemSorter {
    public:
        ItemSorter();
        void add(Item *items, int n);
        int size();
        void sort(int n);
        void setQuery(std::string query);
        void calcHeuristics();
        std::vector<Item>& getItems();
        int copyItems(Item *buffer, int idx, int n);

    private:
        void calcHeuristics(const char *query, bool skipEmpty);
        void calcHeuristics(const char *query, bool skipEmpty,
                                          int start, int end);

        int m_heuristicIdx;
        int m_lastSize;
        int m_sortIdx;

        std::vector<Item> m_items;
        bool m_wakeUp;
        bool m_isSorted;
        std::string m_query;
        bool m_queryChanged;
        bool m_queryDeleted;
        std::mutex m_mut;
};

#endif
