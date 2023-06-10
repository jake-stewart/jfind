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
    switch (Config::instance().caseSensitivity) {
        case CASE_SENSITIVE:
            m_caseSensitive = true;
            break;
        case CASE_INSENSITIVE:
            m_caseSensitive = false;
            break;
        case SMART_CASE:
            m_caseSensitive = false;
            for (char c : query) {
                if (isupper(c)) {
                    m_caseSensitive = true;
                    break;
                }
            }
            break;
    }

    return true;
}

int ItemExactMatcher::calculateScore(Item *item) {
    // todo: add better heuristic
    if (m_caseSensitive) {
        if (strcasestr(item->text, m_query.c_str())) {
            return INT_MAX - strlen(item->text);
        }
        else {
            return BAD_HEURISTIC;
        }
    }
    else {
        if (strstr(item->text, m_query.c_str())) {
            return INT_MAX - strlen(item->text);
        }
        else {
            return BAD_HEURISTIC;
        }
    }
}

