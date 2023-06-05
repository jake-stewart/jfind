#include "../include/item_matcher.hpp"
#include "../include/item.hpp"
#include <climits>

#define isupper(c) (c >= 'A' && c <= 'Z')
#define islower(c) (c >= 'a' && c <= 'z')
#define isdigit(c) (c >= '0' && c <= '9')
#define isalpha(c) (isupper(c) || islower(c))
#define tolower(c) (isupper(c) ? c + 32 : c)

const int MATCH_BONUS = 1;
const int BOUNDARY_BONUS = 100;
const int NEW_WORD_BONUS = 101;
const int CONSECUTIVE_BONUS = 200;
const int DISTANCE_PENALTY = -50;

inline int boundaryScore(const char *c) {
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

int ItemMatcher::calc(const char *text, std::vector<std::string>& queries) {
    int total = 0;
    for (std::string& query : queries) {
        int score = matchStart(text, query.c_str());
        if (score == BAD_HEURISTIC) return BAD_HEURISTIC;
        total += score;
    }
    return total;
}

int ItemMatcher::matchStart(const char *tp, const char *qp) {
    int maxScore = BAD_HEURISTIC;
    int first = BOUNDARY_BONUS;
    int depth = 0;
    while (*tp) {
        if (tolower(*tp) == *qp) {
            int boundary = boundaryScore(tp);
            int score = 0;
            if (*(qp + 1)) {
                score = match(tp + 1, qp + 1, 1, boundary > 0, &depth);
                if (score == BAD_HEURISTIC) return maxScore;
            }
            score += MATCH_BONUS + std::max(boundary, first);
            maxScore = std::max(score, maxScore);
        }
        tp++;
        first = 0;
    }
    return maxScore;
}

int ItemMatcher::match(const char *tp, const char *qp, int dist, bool consec,
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
