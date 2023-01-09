#include "../include/item_matcher.hpp"
// #include "../include/util.hpp"
#include <climits>

// const int SCORE_MODIFIER = -15;
// const int START_LINE_BONUS = 12 * SCORE_MODIFIER;
// const int START_WORD_BONUS = 12 * SCORE_MODIFIER;
// const int CONSECUTIVE_BONUS = 3 * SCORE_MODIFIER;
// const int LETTER_MATCH_BONUS = 1 * SCORE_MODIFIER;
// const int CONSECUTIVE_SHIFT = 4;
// const int DISTANCE_PENALTY = 100;

const int START_LINE_BONUS = -100;
const int START_WORD_BONUS = -100;
const int CONSECUTIVE_BONUS = -10;
const int LETTER_MATCH_BONUS = -1;
const int CONSECUTIVE_SHIFT = 10;
const int DISTANCE_PENALTY = 25;

int ItemMatcher::calc(const char *text, std::vector<std::string>& queries) {
    int total = 0;
    for (std::string& query : queries) {
        int score = matchStart(text, query.c_str());
        if (score == INT_MAX) {
            return INT_MAX;
        }
        total += score;
    }
    return total;
}

int ItemMatcher::matchStart(const char *tp, const char *qp) {
    int minScore = INT_MAX;

    if (!(*tp)) {
        return INT_MAX;
    }

    if (tolower(*tp) == *qp) {
        if (*(qp + 1)) {
            int score = match(tp + 1, qp + 1, 1, 1);
            if (score == INT_MAX) {
                return INT_MAX;
            }
            score += START_LINE_BONUS;
            minScore = score;
        }
        else {
            return START_LINE_BONUS;
        }
    }

    int score = match(tp + 1, qp, 0, 0);
    if (score != INT_MAX && score < minScore) {
        minScore = score;
    }

    return minScore;
}

int ItemMatcher::getLetterScore(const char *tp, int consecutive) {
    if (consecutive) {
        consecutive += CONSECUTIVE_SHIFT;
    }

    int acceptScore;
    switch (*tp) {
        case 'a' ... 'z':
            acceptScore = isalpha(*(tp - 1))
                ? LETTER_MATCH_BONUS
                : START_WORD_BONUS;
            break;
        case 'A' ... 'Z':
            acceptScore = (islower(*(tp + 1)) || !isupper(*(tp - 1)))
                ? START_WORD_BONUS
                : LETTER_MATCH_BONUS;
            break;
        case '0' ... '9':
            acceptScore = isdigit(*(tp - 1))
                ? LETTER_MATCH_BONUS
                : START_WORD_BONUS;
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
    int minScore = INT_MAX;

    while (*tp) {
        if (distance) {
            distance += isWordStart(tp);
        }

        if (tolower(*tp) == *qp) {
            if (!(*(qp + 1))) {
                int score = getLetterScore(tp, consecutive) + distance
                    * DISTANCE_PENALTY;
                if (score < minScore) {
                    minScore = score;
                }
            }
            else {
                int score;
                if (islower(*tp)) {
                    if (consecutive == 0 && isalpha(*(tp - 1))) {
                        score = match(tp + 1, qp + 1, distance + 1, 0);
                    }
                    else if (isupper(*(tp + 1))) {
                        score = match(tp + 1, qp + 1, distance + 1, 0);
                    }
                    else {
                        score = match(tp + 1, qp + 1, distance + 1,
                                consecutive + 1);
                    }
                }
                else {
                    score = match(tp + 1, qp + 1, distance + 1,
                            consecutive + 1);
                }
                if (score != INT_MAX) {
                    score += getLetterScore(tp, consecutive) + distance
                        * DISTANCE_PENALTY;
                    if (score < minScore) {
                        minScore = score;
                    }
                }
            }
        }

        consecutive = 0;

        // if (distance) {
        //     distance++;
        // }

        tp++;
    }

    if (minScore == INT_MAX) {
        return INT_MAX;
    }

    return minScore;
}
