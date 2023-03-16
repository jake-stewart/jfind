#include "../include/item_cache.hpp"

ItemCache::ItemCache(ItemSorter *sorter) {
    m_sorter = sorter;
    m_cache.setDatasource([this] (Item *buffer, int idx, int n) {
        return m_sorter->copyItems(buffer, idx, n);
    });
}

void ItemCache::refresh() {
    m_cachedSize = m_sorter->size();
    m_cache.refresh();
}

Item* ItemCache::get(int i) {
    return m_cache.get(i);
}

int ItemCache::size() {
    return m_cachedSize;
}

int ItemCache::getReserve() {
    return m_cache.getReserve();
}

void ItemCache::setReserve(int n) {
    m_cache.setReserve(n);
}
