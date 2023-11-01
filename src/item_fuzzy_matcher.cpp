#include "../include/item_fuzzy_matcher.hpp"
#include "../include/item.hpp"
#include "../include/util.hpp"
#include "../include/config.hpp"
#include <climits>
#include <cstring>

#define isupper(c) (c >= 'A' && c <= 'Z')
#define islower(c) (c >= 'a' && c <= 'z')
#define isdigit(c) (c >= '0' && c <= '9')
#define isalpha(c) (isupper(c) || islower(c))
#define tolower(c) (isupper(c) ? c + 32 : c)

static const int MATCH_BONUS = 1;
static const int INNER_WORD_BOUNDARY_BONUS = 100; // inner(W)ord
static const int NEW_WORD_BONUS = 101;            // new-(w)ord
static const int SEPARATED_WORD_BONUS = 102;      // separated/(w)ord
static const int START_LINE_BONUS = 103;          // (s)tart of line

static const int CONSECUTIVE_BONUS = 200;
static const int DISTANCE_PENALTY = -50;

static inline int boundaryScore(const char *c) {
    switch (*c) {
        case 'a' ... 'z':
            switch (*(c - 1)) {
                case 'a' ... 'z':
                case 'A' ... 'Z':
                    return 0;
                case '0' ... '9':
                    return INNER_WORD_BOUNDARY_BONUS;
                case '/': case '|': case ',': case ':': case ';':
                    return SEPARATED_WORD_BONUS;
                default:
                    return NEW_WORD_BONUS;
            }
        case 'A' ... 'Z':
            switch (*(c - 1)) {
                case 'a' ... 'z':
                    return INNER_WORD_BOUNDARY_BONUS;
                case 'A' ... 'Z':
                    return islower(*(c + 1)) * INNER_WORD_BOUNDARY_BONUS;
                case '0' ... '9':
                    return INNER_WORD_BOUNDARY_BONUS;
                case '/': case '|': case ',': case ':': case ';':
                    return SEPARATED_WORD_BONUS;
                default:
                    return NEW_WORD_BONUS;
            }
        case '0' ... '9':
            switch (*(c - 1)) {
                case '0' ... '9':
                    return 0;
                case 'a' ... 'z':
                case 'A' ... 'Z':
                    return INNER_WORD_BOUNDARY_BONUS;
                case '_':
                case '-':
                    return NEW_WORD_BONUS;
                default:
                    return SEPARATED_WORD_BONUS;
            }
        default:
            return 0;
    }
}

bool ItemFuzzyMatcher::setQuery(std::string query) {
    m_queries = split(toLower(query), ' ');
    return true;
}

bool ItemFuzzyMatcher::requiresFullRescore() {
    return false;
}

int ItemFuzzyMatcher::calculateScore(Item *item) {
    if (!*item->text) {
        return BAD_HEURISTIC;
    }

    int total = 0;
    for (const std::string &query : m_queries) {
        int score = matchStart(item->text, query.c_str());
        if (score == BAD_HEURISTIC) {
            return BAD_HEURISTIC;
        }
        total += score;
    }
    switch (Config::instance().lengthPreference) {
        case SHORT:
            return total - strlen(item->text);
        case LONG:
            return total + strlen(item->text);
        case NONE:
            return total;
    }
}

int ItemFuzzyMatcher::matchStart(const char *tp, const char *qp) {
    int maxScore = BAD_HEURISTIC;
    int depth = 0;
    if (tolower(*tp) == *qp) {
        int score = 0;
        if (*(qp + 1)) {
            score = match(tp + 1, qp + 1, 1, true, &depth);
            if (score == BAD_HEURISTIC) {
                return maxScore;
            }
        }
        maxScore = score + MATCH_BONUS + START_LINE_BONUS;
    }
    tp++;
    while (*tp) {
        if (tolower(*tp) == *qp) {
            int boundary = boundaryScore(tp);
            int score = 0;
            if (*(qp + 1)) {
                score = match(tp + 1, qp + 1, 1, boundary > 0, &depth);
                if (score == BAD_HEURISTIC) {
                    return maxScore;
                }
            }
            score += MATCH_BONUS + boundary;
            maxScore = std::max(score, maxScore);
        }
        tp++;
    }
    return maxScore;
}

int ItemFuzzyMatcher::match(
    const char *tp, const char *qp, int dist, bool consec, int *depth
) {
    int maxScore = BAD_HEURISTIC;
    while (*tp) {
        int boundary = boundaryScore(tp);
        dist += boundary > 0;
        if (tolower(*tp) == *qp) {
            int score = 0;
            if (*(qp + 1)) {
                score = match(tp + 1, qp + 1, dist, consec || boundary, depth);
                if (score == BAD_HEURISTIC) {
                    return maxScore;
                }
            }
            score += MATCH_BONUS + boundary + consec * CONSECUTIVE_BONUS +
                dist * DISTANCE_PENALTY;
            maxScore = std::max(score, maxScore);
            if (++(*depth) > 1024) {
                return maxScore;
            }
        }
        tp++;
        consec = false;
    }
    return maxScore;
}
