#ifndef JFIND_STRATEGY_HPP
#define JFIND_STRATEGY_HPP

#include "item_cache.hpp"
#include "item_sorter.hpp"
#include "file_item_reader.hpp"
#include "process_item_reader.hpp"
#include <thread>

class JfindStrategy
{
protected:
    ItemCache &m_itemCache;

public:
    JfindStrategy(ItemCache &itemCache);
    virtual void start() = 0;
    virtual bool isThreadsafeReading() = 0;
};

class FuzzyFindStrategy : public JfindStrategy
{
    FileItemReader m_itemReader;
    ItemSorter m_itemSorter;

public:
    FuzzyFindStrategy(ItemCache &itemCache);
    void start() override;
    bool isThreadsafeReading() override;
};

class InteractiveCommandStrategy : public JfindStrategy
{
    ProcessItemReader m_itemReader;

public:
    InteractiveCommandStrategy(ItemCache &itemCache);
    void start() override;
    bool isThreadsafeReading() override;
};

#endif
