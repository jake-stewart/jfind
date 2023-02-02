#include "../include/item_matcher.hpp"
#include "../include/item.hpp"
#include <climits>

const int START_LINE_BONUS = 100;
const int START_WORD_BONUS = 100;
const int CONSECUTIVE_BONUS = 90;
const int LETTER_MATCH_BONUS = 1;
const int DISTANCE_PENALTY = -25;

#define isupper(c) (c >= 'A' && c <= 'Z')
#define islower(c) (c >= 'a' && c <= 'z')
#define isdigit(c) (c >= '0' && c <= '9')
#define tolower(c) (isupper(c) ? c + 32 : c)

inline bool isWordStart(const char *c) {
    return (islower(*c)) && !isalpha(*(c - 1))
        || (isupper(*c)) && (!isupper(*(c - 1)) || islower(*(c + 1)))
        || (isdigit(*c)) && !isdigit(*(c - 1));
}

int ItemMatcher::calc(const char *text, std::vector<std::string>& queries) {
    int total = 0;
    for (std::string& query : queries) {
        int score = matchStart(text, query.c_str());
        if (score == BAD_HEURISTIC) {
            return BAD_HEURISTIC;
        }
        total += score;
    }
    return total;
}

int ItemMatcher::matchStart(const char *tp, const char *qp) {
    int maxScore = BAD_HEURISTIC;
    bool startOfLine = true;
    while (*tp) {
        bool wordStart = isWordStart(tp);
        if (tolower(*tp) == *qp) {
            int score = 0;
            if (*(qp + 1)) {
                score = match(tp + 1, qp + 1, 1, wordStart);
                if (score == BAD_HEURISTIC) {
                    return maxScore;
                }
            }
            score += LETTER_MATCH_BONUS
                + (wordStart | startOfLine) * START_WORD_BONUS;
            maxScore = std::max(score, maxScore);
            tp++;
        }
        else {
            do tp++;
            while (tolower(*tp) != *qp && *tp);
        }
        startOfLine = false;
    }
    return maxScore;
}

int ItemMatcher::match(const char *tp, const char *qp, int distance,
        bool consecutive)
{
    int maxScore = BAD_HEURISTIC;
    while (*tp) {
        bool wordStart = isWordStart(tp);
        distance += wordStart;
        if (tolower(*tp) == *qp) {
            int score = 0;
            if (*(qp + 1)) {
                score = match(tp + 1, qp + 1, distance, consecutive | wordStart);
                if (score == BAD_HEURISTIC) {
                    return maxScore;
                }
            }
            score += (LETTER_MATCH_BONUS + wordStart * START_WORD_BONUS)
                + (consecutive * CONSECUTIVE_BONUS)
                + (distance * DISTANCE_PENALTY);
            maxScore = std::max(score, maxScore);
            tp++;
        }
        else {
            do tp++;
            while (tolower(*tp) != *qp && *tp);
        }
        consecutive = false;
    }
    return maxScore;
}
