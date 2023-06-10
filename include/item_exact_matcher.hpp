#ifndef ITEM_EXACT_MATCHER_HPP
#define ITEM_EXACT_MATCHER_HPP

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

public:
    bool requiresFullRescore() override;
    int calculateScore(Item *item) override;
    bool setQuery(std::string query) override;
};

#endif
