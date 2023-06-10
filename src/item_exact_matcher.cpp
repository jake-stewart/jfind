#include "../include/item_exact_matcher.hpp"
#include "../include/item.hpp"
#include "../include/config.hpp"
#include <climits>
#include <regex>
#include <cstdio>
#include <cstring>


bool ItemExactMatcher::requiresFullRescore() {
    return false;
}

bool ItemExactMatcher::setQuery(std::string query) {
    m_query = query;
    bool caseSensitive;
    switch (Config::instance().caseSensitivity) {
        case CASE_SENSITIVE:
            caseSensitive = true;
            break;
        case CASE_INSENSITIVE:
            caseSensitive = false;
            break;
        case SMART_CASE:
            caseSensitive = false;
            for (char c : query) {
                if (isupper(c)) {
                    m_caseSensitive = true;
                    break;
                }
            }
            break;
    }

    if (caseSensitive) {
        m_comparison = strcasestr;
    }
    else {
        m_comparison = strstr;
    }

    return true;
}

int ItemExactMatcher::calculateScore(Item *item) {
    // todo: add better heuristic
    if (m_comparison(item->text, m_query.c_str())) {
        return INT_MAX - strlen(item->text);
    }
    else {
        return BAD_HEURISTIC;
    }
}

