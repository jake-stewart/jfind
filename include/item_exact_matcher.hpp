#ifndef ITEM_EXACT_MATCHER_HPP
#define ITEM_EXACT_MATCHER_HPP

#include "length_preference.hpp"
#include "item_matcher.hpp"
#include <climits>
#include <regex>
#include <string>
#include <vector>

class ItemExactMatcher : public ItemMatcher
{
    std::string m_query;
    LengthPreference m_lengthPreference;
    bool m_caseSensitive;

public:
    bool requiresFullRescore() override;
    int calculateScore(Item *item) override;
    bool setQuery(std::string query) override;
};

#endif
