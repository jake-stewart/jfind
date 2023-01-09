#include "../include/item_matcher.hpp"
#include <climits>

const int START_LINE_BONUS = 100;
const int START_WORD_BONUS = 100;
const int CONSECUTIVE_BONUS = 10;
const int LETTER_MATCH_BONUS = 1;
const int CONSECUTIVE_SHIFT = 10;
const int DISTANCE_PENALTY = -25;

int ItemMatcher::calc(const char *text, std::vector<std::string>& queries) {
    int total = 0;
    for (std::string& query : queries) {
        int score = matchStart(text, query.c_str());
        if (score == -INT_MAX) {
            return score;
        }
        total += score;
    }
    return total;
}

int ItemMatcher::matchStart(const char *tp, const char *qp) {
    int maxScore = -INT_MAX;

    if (!(*tp)) {
        return maxScore;
    }

    if (tolower(*tp) == *qp) {
        if (!*(qp + 1)) {
            return START_LINE_BONUS;
        }
        maxScore = match(tp + 1, qp + 1, 1, 1);
        if (maxScore == -INT_MAX) {
            return maxScore;
        }
        maxScore += START_LINE_BONUS;
    }

    int score = match(tp + 1, qp, 0, 0);
    return score > maxScore ? score : maxScore;
}

int ItemMatcher::letterScore(const char *tp, int consecutive) {
    if (consecutive) {
        consecutive += CONSECUTIVE_SHIFT;
    }

    int acceptScore;
    switch (*tp) {
        case 'a' ... 'z':
            acceptScore = isalpha(*(tp - 1))
                ? LETTER_MATCH_BONUS : START_WORD_BONUS;
            break;
        case 'A' ... 'Z':
            acceptScore = (islower(*(tp + 1)) || !isupper(*(tp - 1)))
                ? START_WORD_BONUS : LETTER_MATCH_BONUS;
            break;
        case '0' ... '9':
            acceptScore = isdigit(*(tp - 1))
                ? LETTER_MATCH_BONUS : START_WORD_BONUS;
            break;
        default:
            acceptScore = LETTER_MATCH_BONUS;
            break;
    }

    return acceptScore + consecutive * CONSECUTIVE_BONUS;
}

bool ItemMatcher::isWordStart(const char *tp) {
    switch (*tp) {
        case 'a' ... 'z':
            return !isalpha(*(tp - 1));
        case 'A' ... 'Z':
            return islower(*(tp + 1)) || !isupper(*(tp - 1));
        case '0' ... '9':
            return !isdigit(*(tp - 1));
        default:
            return false;
    }
}

int ItemMatcher::match(const char *tp, const char *qp, int distance,
        int consecutive)
{
    int maxScore = -INT_MAX;

    while (*tp) {
        if (distance) {
            distance += isWordStart(tp);
        }

        if (tolower(*tp) == *qp) {
            if (!(*(qp + 1))) {
                int score = letterScore(tp, consecutive)
                    + distance * DISTANCE_PENALTY;
                if (score > maxScore) {
                    maxScore = score;
                }
            }
            else {
                bool resetConsecutive = !consecutive && islower(*tp)
                    && isalpha(*(tp - 1));

                int score = match(tp + 1, qp + 1, distance + 1,
                        resetConsecutive ? 0 : consecutive + 1);

                if (score != -INT_MAX) {
                    score += letterScore(tp, consecutive)
                        + distance * DISTANCE_PENALTY;
                    if (score > maxScore) {
                        maxScore = score;
                    }
                }
            }
        }

        consecutive = 0;
        tp++;
    }

    return maxScore;
}
