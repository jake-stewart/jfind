#include "../include/item_matcher.hpp"
#include <climits>

const int START_LINE_BONUS = 100;
const int START_WORD_BONUS = 100;
const int CONSECUTIVE_BONUS = 10;
const int LETTER_MATCH_BONUS = 1;
const int CONSECUTIVE_SHIFT = 10;
const int DISTANCE_PENALTY = -25;

#define islower(c) (c >= 'a' && c <= 'z')
#define isupper(c) (c >= 'A' && c <= 'Z')
#define isdigit(c) (c >= '0' && c <= '9')
#define tolower(c) (isupper(c) ? c + 32 : c)

inline bool isWordStart(const char *c) {
    if (islower(*c)) return !isalpha(*(c - 1));
    if (isupper(*c)) return !isupper(*(c - 1)) || islower(*(c + 1));
    if (isdigit(*c)) return !isdigit(*(c - 1));
    return false;
}

int ItemMatcher::calc(const char *text, std::vector<std::string>& queries) {
    int total = 0;
    for (std::string& query : queries) {
        int score = matchStart(text, query.c_str());
        if (score == -INT_MAX) return score;
        total += score;
    }
    return total;
}

int ItemMatcher::matchStart(const char *tp, const char *qp) {
    int maxScore = -INT_MAX;
    while (*tp) {
        bool wordStart = isWordStart(tp);
        if (tolower(*tp) == *qp) {
            int score = 0;
            if (*(qp + 1)) {
                score = match(tp + 1, qp + 1, 1, wordStart);
                if (score == -INT_MAX) {
                    break;
                }
            }
            score += (LETTER_MATCH_BONUS + wordStart * START_WORD_BONUS);
            if (score > maxScore) {
                maxScore = score;
            }
        }
        tp++;
    }
    return maxScore;
}

int ItemMatcher::match(const char *tp, const char *qp, int distance, int consecutive) {
    int maxScore = -INT_MAX;
    while (*tp) {
        bool wordStart = isWordStart(tp);
        distance += wordStart;
        if (tolower(*tp) == *qp) {
            int score = 0;
            if (*(qp + 1)) {
                score = match(tp + 1, qp + 1, distance, consecutive ? consecutive : wordStart);
                if (score == -INT_MAX) {
                    break;
                }
            }
            score += (LETTER_MATCH_BONUS + wordStart * START_WORD_BONUS)
                + ((consecutive > 0) * (consecutive + CONSECUTIVE_SHIFT) * CONSECUTIVE_BONUS)
                + (distance * DISTANCE_PENALTY);
            if (score > maxScore) {
                maxScore = score;
            }
        }
        consecutive = 0;
        tp++;
    }
    return maxScore;
}
