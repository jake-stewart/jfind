#include "../include/item_cache.hpp"

void ItemCache::setSizeCallback(std::function<int()> sizeCallback) {
    m_sizeCallback = sizeCallback;
}

void ItemCache::setItemsCallback(
    std::function<int(Item *buffer, int idx, int n)> itemsCallback
) {
    m_cache.setDatasource(itemsCallback);
}

void ItemCache::refresh() {
    m_cachedSize = m_sizeCallback();
    m_cache.refresh();
}

Item* ItemCache::get(int i) {
    return m_cache.get(i);
}

int ItemCache::size() const {
    return m_cachedSize;
}

int ItemCache::getReserve() const {
    return m_cache.getReserve();
}

void ItemCache::setReserve(int n) {
    m_cache.setReserve(n);
}
