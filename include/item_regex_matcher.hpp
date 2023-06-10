#ifndef ITEM_REGEX_MATCHER_HPP
#define ITEM_REGEX_MATCHER_HPP

#include "item_matcher.hpp"
#include <climits>
#include <regex>
#include <string>
#include <vector>

enum RegexCaseSensitivity {
    CASE_SENSITIVE,
    CASE_INSENSITIVE,
    SMART_CASE,
};

class ItemRegexMatcher : public ItemMatcher
{
    std::regex m_pattern;

public:
    bool requiresFullRescore() override;
    int calculateScore(Item *item) override;
    bool setQuery(std::string query) override;
};

#endif
