#ifndef ITEM_CACHE_HPP
#define ITEM_CACHE_HPP

#include "item.hpp"
#include "item_sorter.hpp"
#include "sliding_cache.hpp"

class ItemCache {
    public:
        ItemCache(ItemSorter *sorter);
        void refresh();
        Item* get(int i);
        int size() const;
        int getReserve() const;
        void setReserve(int n);

    private:
        SlidingCache<Item> m_cache;
        int m_cachedSize;
        ItemSorter *m_sorter;
};

#endif
