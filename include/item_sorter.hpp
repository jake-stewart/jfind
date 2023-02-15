#ifndef ITEM_SORTER_HPP
#define ITEM_SORTER_HPP

#include "item.hpp"
#include <vector>
#include <fstream>
#include <mutex>

class ItemSorter {
    public:
        ItemSorter();
        void add(Item *items, int n);
        int size();
        void sort(int n);
        void setQuery(std::string query);
        void calcHeuristics(bool *cancel);
        std::vector<Item>& getItems();
        int copyItems(Item *buffer, int idx, int n);

    private:
        void calcHeuristics(const char *query, bool newItems,
                            int start, int end, bool *cancel);

        int m_heuristicIdx;
        int m_lastSize;
        int m_sortIdx;

        std::vector<Item> m_items;
        bool m_wakeUp;
        bool m_isSorted;
        std::string m_query;
        bool m_queryChanged;
        std::mutex m_mut;
};

#endif
