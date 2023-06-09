#ifndef ITEM_REGEX_MATCHER_HPP
#define ITEM_REGEX_MATCHER_HPP

#include "item_matcher.hpp"
#include <climits>
#include <regex>
#include <string>
#include <vector>

class ItemRegexMatcher : public ItemMatcher
{
    std::regex m_pattern;

public:
    bool requiresFullRescore() override;
    int calculateScore(const char *text) override;
    bool setQuery(std::string query) override;
};

#endif
