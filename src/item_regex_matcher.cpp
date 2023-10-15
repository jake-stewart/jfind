#include "../include/item_regex_matcher.hpp"
#include "../include/config.hpp"
#include "../include/item.hpp"
#include <climits>
#include <cstring>
#include <regex>

bool ItemRegexMatcher::requiresFullRescore() {
    return true;
}
bool ItemRegexMatcher::setQuery(std::string query) {
    bool caseSensitive;

    m_lengthPreference = Config::instance().lengthPreference;

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
    catch (const std::regex_error &error) {
        return false;
    }
}

int ItemRegexMatcher::calculateScore(Item *item) {
    if (std::regex_search(item->text, m_pattern)) {
        switch (m_lengthPreference) {
            case SHORT:
                return -strlen(item->text);
            case LONG:
                return strlen(item->text);
            case NONE:
                return 0;
        }
    }
    return BAD_HEURISTIC;
}
