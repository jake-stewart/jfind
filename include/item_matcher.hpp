#ifndef ITEM_MATCHER_HPP
#define ITEM_MATCHER_HPP

#include <string>

enum MatcherType {
    FUZZY_MATCHER,
    REGEX_MATCHER
};

class ItemMatcher
{
public:
    virtual bool setQuery(std::string query) = 0;
    virtual int calculateScore(const char *query) = 0;
    virtual bool requiresFullRescore() = 0;
};

#endif
