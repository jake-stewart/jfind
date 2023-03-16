#include "../include/item_matcher.hpp"
#include "../include/item.hpp"
#include <climits>

#define isupper(c) (c >= 'A' && c <= 'Z')
#define islower(c) (c >= 'a' && c <= 'z')
#define isdigit(c) (c >= '0' && c <= '9')
#define tolower(c) (isupper(c) ? c + 32 : c)

inline bool isWordStart(const char *c) {
    return (islower(*c)) && !isalpha(*(c - 1))
        || (isupper(*c)) && (!isupper(*(c - 1)) || islower(*(c + 1)))
        || (isdigit(*c)) && !isdigit(*(c - 1));
}

const int MATCH_BONUS = 1;
const int BOUNDARY_BONUS = 100;
const int CONSECUTIVE_BONUS = 200;
const int DISTANCE_PENALTY = -50;

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
    bool first = true;
    int depth = 0;
    while (*tp) {
        if (tolower(*tp) == *qp) {
            bool boundary = isWordStart(tp);
            int score = 0;
            if (*(qp + 1)) {
                score = match(tp + 1, qp + 1, 1, boundary, &depth);
                if (score == BAD_HEURISTIC) return maxScore;
            }
            score += MATCH_BONUS + (boundary | first) * BOUNDARY_BONUS;
            maxScore = std::max(score, maxScore);
        }
        tp++;
        first = false;
    }
    return maxScore;
}

int ItemMatcher::match(const char *tp, const char *qp, int dist, bool consec, int *depth) {
    int maxScore = BAD_HEURISTIC;
    while (*tp) {
        bool boundary = isWordStart(tp);
        dist += boundary;
        if (tolower(*tp) == *qp) {
            int score = 0;
            if (*(qp + 1)) {
                score = match(tp + 1, qp + 1, dist, consec | boundary, depth);
                if (score == BAD_HEURISTIC) return maxScore;
            }
            score += MATCH_BONUS + boundary * BOUNDARY_BONUS
                + consec * CONSECUTIVE_BONUS + dist * DISTANCE_PENALTY;
            maxScore = std::max(score, maxScore);
            if (++(*depth) > 100) return maxScore;
        }
        tp++;
        consec = false;
    }
    return maxScore;
}
