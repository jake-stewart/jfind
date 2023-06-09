#include "../include/item_regex_matcher.hpp"
#include "../include/item.hpp"
#include <climits>
#include <regex>


bool ItemRegexMatcher::requiresFullRescore() {
    return true;
}

bool ItemRegexMatcher::setQuery(std::string query) {
    try {
        m_pattern = std::regex(query);
        return true;
    }
    catch (const std::regex_error& error) {
        return false;
    }
}

int ItemRegexMatcher::calculateScore(const char *text) {
    if (std::regex_search(text, m_pattern)) {
        return 0;
    }
    return -INT_MAX;
}

