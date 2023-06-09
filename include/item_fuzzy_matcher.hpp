#ifndef ITEM_FUZZY_MATCHER_HPP
#define ITEM_FUZZY_MATCHER_HPP

#include "item_matcher.hpp"
#include <string>
#include <vector>

class ItemFuzzyMatcher : public ItemMatcher
{
    std::vector<std::string> m_queries;

public:
    bool requiresFullRescore() override;
    bool setQuery(std::string query) override;
    int calculateScore(const char *text) override;

private:
    int matchStart(const char *tp, const char *qp);
    int match(
        const char *tp, const char *qp, int distance, bool consecutive,
        int *depth
    );
};

#endif
