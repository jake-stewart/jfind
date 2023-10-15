#include "../include/item_exact_matcher.hpp"
#include "../include/config.hpp"
#include "../include/item.hpp"
#include <climits>
#include <cstdio>
#include <cstring>
#include <regex>

bool ItemExactMatcher::requiresFullRescore() {
    return false;
}

bool ItemExactMatcher::setQuery(std::string query) {
    m_query = query;

    m_lengthPreference = Config::instance().lengthPreference;

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
        if (!strstr(item->text, m_query.c_str())) {
            return BAD_HEURISTIC;
        }
    }
    else if (!strcasestr(item->text, m_query.c_str())) {
        return BAD_HEURISTIC;
    }

    switch (m_lengthPreference) {
        case SHORT:
            return -strlen(item->text);
        case LONG:
            return strlen(item->text);
        case NONE:
            return 0;
    }
}
