#include "../include/item_regex_matcher.hpp"
#include "../include/item.hpp"
#include "../include/config.hpp"
#include <climits>
#include <regex>


bool ItemRegexMatcher::requiresFullRescore() {
    return true;
}
bool ItemRegexMatcher::setQuery(std::string query) {
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
                    caseSensitive = true;
                    break;
                }
            }
            break;
    }

    try {
        if (caseSensitive) {
            m_pattern = std::regex(query);
        }
        else {
            m_pattern = std::regex(query, std::regex_constants::icase);
        }
        return true;
    }
    catch (const std::regex_error& error) {
        return false;
    }
}

int ItemRegexMatcher::calculateScore(Item *item) {
    if (std::regex_search(item->text, m_pattern)) {
        return INT_MAX - item->index;
    }
    return BAD_HEURISTIC;
}

