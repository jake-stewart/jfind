#ifndef ITEM_MATCHER_HPP
#define ITEM_MATCHER_HPP

#include "item.hpp"
#include <string>

enum MatcherType {
    FUZZY_MATCHER,
    REGEX_MATCHER,
    EXACT_MATCHER,
};

class ItemMatcher
{
public:
    virtual bool setQuery(std::string query) = 0;
    virtual int calculateScore(Item *item) = 0;
    virtual bool requiresFullRescore() = 0;
    virtual ~ItemMatcher() {}
};

#endif
