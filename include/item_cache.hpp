#ifndef ITEM_CACHE_HPP
#define ITEM_CACHE_HPP

#include "item.hpp"
#include "sliding_cache.hpp"

class ItemCache
{
public:
    ItemCache() {}
    void refresh();
    Item *get(int i);
    int size() const;
    int getReserve() const;
    void setReserve(int n);

    void setSizeCallback(std::function<int()> sizeCallback);
    void setItemsCallback(
        std::function<int(Item *buffer, int idx, int n)> itemsCallback
    );

private:
    SlidingCache<Item> m_cache;
    std::function<int()> m_sizeCallback;
    int m_cachedSize;
};

#endif
