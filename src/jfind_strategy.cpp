#include "../include/jfind_strategy.hpp"

#include "../include/config.hpp"
#include "../include/file_item_reader.hpp"
#include "../include/item_sorter.hpp"
#include "../include/process_item_reader.hpp"
#include "../include/item_fuzzy_matcher.hpp"
#include "../include/item_regex_matcher.hpp"
#include "../include/item_exact_matcher.hpp"

JfindStrategy::JfindStrategy(ItemCache &itemCache) : m_itemCache(itemCache) {}

FuzzyFindStrategy::FuzzyFindStrategy(ItemCache &itemCache) :
    JfindStrategy(itemCache),
    m_itemSorter(Config::instance().query),
    m_itemReader(stdin) {}

bool FuzzyFindStrategy::isThreadsafeReading() {
    return false;
}

void FuzzyFindStrategy::start() {
    Config &config = Config::instance();

    switch (Config::instance().matcher) {
        case FUZZY_MATCHER:
            m_itemSorter.setMatcher(new ItemFuzzyMatcher());
            break;
        case REGEX_MATCHER:
            m_itemSorter.setMatcher(new ItemRegexMatcher());
            break;
        case EXACT_MATCHER:
            m_itemSorter.setMatcher(new ItemExactMatcher());
            break;
    }

    m_itemCache.setItemsCallback([this](Item *buffer, int idx, int n) {
        return m_itemSorter.copyItems(buffer, idx, n);
    });
    m_itemCache.setSizeCallback([this]() {
        return m_itemSorter.size();
    });

    m_itemSorterThread = new std::thread(&ItemSorter::start, &m_itemSorter);
    m_itemReaderThread = new std::thread(&FileItemReader::start, &m_itemReader);
}

InteractiveCommandStrategy::InteractiveCommandStrategy(ItemCache &itemCache) :
    JfindStrategy(itemCache),
    m_itemReader(Config::instance().command, Config::instance().query) {}

bool InteractiveCommandStrategy::isThreadsafeReading() {
    return true;
}

void InteractiveCommandStrategy::start() {
    close(STDIN_FILENO);
    Config &config = Config::instance();

    m_itemCache.setItemsCallback([this](Item *buffer, int idx, int n) {
        return m_itemReader.copyItems(buffer, idx, n);
    });

    m_itemCache.setSizeCallback([this]() {
        return m_itemReader.size();
    });

    m_itemReaderThread = new std::thread(&ProcessItemReader::start, &m_itemReader);
}

