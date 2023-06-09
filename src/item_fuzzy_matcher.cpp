#include "../include/item_fuzzy_matcher.hpp"
#include "../include/item.hpp"
#include "../include/util.hpp"
#include <climits>

#define isupper(c) (c >= 'A' && c <= 'Z')
#define islower(c) (c >= 'a' && c <= 'z')
#define isdigit(c) (c >= '0' && c <= '9')
#define isalpha(c) (isupper(c) || islower(c))
#define tolower(c) (isupper(c) ? c + 32 : c)

static const int MATCH_BONUS = 1;
static const int BOUNDARY_BONUS = 100;
static const int NEW_WORD_BONUS = 101;
static const int CONSECUTIVE_BONUS = 200;
static const int DISTANCE_PENALTY = -50;

static inline int boundaryScore(const char *c) {
    if (islower(*c)) {
        if (!isalpha(*(c - 1))) {
            return NEW_WORD_BONUS;
        }
    }
    else if (isupper(*c)) {
        if (!isalpha(*(c - 1))) {
            return NEW_WORD_BONUS;
        }
        if (!isupper(*(c - 1)) || islower(*(c + 1))) {
            return BOUNDARY_BONUS;
        }
    }
    else if ((isdigit(*c)) && !isdigit(*(c - 1))) {
        return BOUNDARY_BONUS;
    }
    return 0;
}

bool ItemFuzzyMatcher::setQuery(std::string query) {
    m_queries = split(query, ' ');
    return true;
}

bool ItemFuzzyMatcher::requiresFullRescore() {
    return false;
}

int ItemFuzzyMatcher::calculateScore(const char *text) {
    int total = 0;
    for (const std::string& query : m_queries) {
        int score = matchStart(text, query.c_str());
        if (score == BAD_HEURISTIC) return BAD_HEURISTIC;
        total += score;
    }
    return total;
}

int ItemFuzzyMatcher::matchStart(const char *tp, const char *qp) {
    int maxScore = BAD_HEURISTIC;
    int depth = 0;
    if (tolower(*tp) == *qp) {
        int score = 0;
        if (*(qp + 1)) {
            score = match(tp + 1, qp + 1, 1, true, &depth);
            if (score == BAD_HEURISTIC) return maxScore;
        }
        maxScore = score + MATCH_BONUS + BOUNDARY_BONUS;
    }
    tp++;
    while (*tp) {
        if (tolower(*tp) == *qp) {
            int boundary = boundaryScore(tp);
            int score = 0;
            if (*(qp + 1)) {
                score = match(tp + 1, qp + 1, 1, boundary > 0, &depth);
                if (score == BAD_HEURISTIC) return maxScore;
            }
            score += MATCH_BONUS + boundary;
            maxScore = std::max(score, maxScore);
        }
        tp++;
    }
    return maxScore;
}

int ItemFuzzyMatcher::match(const char *tp, const char *qp, int dist, bool consec,
                       int *depth)
{
    int maxScore = BAD_HEURISTIC;
    while (*tp) {
        bool boundary = boundaryScore(tp);
        dist += boundary > 0;
        if (tolower(*tp) == *qp) {
            int score = 0;
            if (*(qp + 1)) {
                score = match(tp + 1, qp + 1, dist, consec || boundary, depth);
                if (score == BAD_HEURISTIC) return maxScore;
            }
            score += MATCH_BONUS + boundary + consec * CONSECUTIVE_BONUS
                + dist * DISTANCE_PENALTY;
            maxScore = std::max(score, maxScore);
            if (++(*depth) > 100) return maxScore;
        }
        tp++;
        consec = false;
    }
    return maxScore;
}
