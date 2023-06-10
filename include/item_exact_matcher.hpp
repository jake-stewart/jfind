#ifndef ITEM_REGEX_MATCHER_HPP
#define ITEM_REGEX_MATCHER_HPP

#include "item_matcher.hpp"
#include "case_sensitivity.hpp"
#include <climits>
#include <regex>
#include <string>
#include <vector>

class ItemExactMatcher : public ItemMatcher
{
    std::string m_query;
    bool m_caseSensitive;
    char* (*m_comparison)(const char*, const char*);

public:
    bool requiresFullRescore() override;
    int calculateScore(Item *item) override;
    bool setQuery(std::string query) override;
};

#endif
