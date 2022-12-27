#include "../include/source_matcher_v3.hpp"
#include "../include/util.hpp"

#include <climits>

int SourceMatcherV3::calc(const char *text, const char **queries, int n_queries) {
    int total = 0;
    for (int i = 0; i < n_queries; i++) {
        int score = match(text, queries[i], true);
        if (!score) {
            return INT_MAX;
        }
        total += score;
    }
    return total;
}

inline char fast_tolower(char c) {
    switch (c) {
        case 'A' ... 'Z':
            return c + 32;
        default:
            return c;
    }
}

inline bool fast_isalpha(char c) {
    switch (c) {
        case 'a' ... 'z':
        case 'A' ... 'Z':
            return true;
        default:
            return false;
    }
}

inline bool fast_islower(char c) {
    switch (c) {
        case 'a' ... 'z':
            return true;
        default:
            return false;
    }
}

inline bool fast_isdigit(char c) {
    switch (c) {
        case '0' ... '9':
            return true;
        default:
            return false;
    }
}

const int SCORE_MODIFIER = -100;
const int START_LINE_BONUS = 3 * SCORE_MODIFIER;
const int START_WORD_BONUS = 2 * SCORE_MODIFIER;
const int LETTER_MATCH_BONUS = 1 * SCORE_MODIFIER;
const int CONSECUTIVE_BONUS = 1 * SCORE_MODIFIER;

int SourceMatcherV3::match(const char *tp, const char *qp, bool is_start) {
    int score = match_recurse(tp, qp, is_start);
    if (score) {
        return score + strlen(tp);
    }
    return 0;
}

int SourceMatcherV3::match_recurse(const char *tp, const char *qp, bool is_start) {
    bool isConsecutive = false;
    int score = 0;
    int min_score = 0;

    if (is_start) {
        if (fast_tolower(*tp) == *qp) {
            int ignore_score = score + match_recurse(tp + 1, qp, false);
            if (ignore_score && ignore_score < min_score) {
                min_score = ignore_score;
            }
            score += START_LINE_BONUS;
            isConsecutive = true;
            qp++;
        }
        tp++;
    }

    while (*tp) {
        if (fast_tolower(*tp) == *qp) {
            int ignore_score = score + match_recurse(tp + 1, qp, false);
            if (ignore_score && ignore_score < min_score) {
                min_score = ignore_score;
            }

            switch (*tp) {
                case 'a' ... 'z':
                    score += fast_isalpha(*(tp - 1))
                        ? LETTER_MATCH_BONUS
                        : START_WORD_BONUS;
                    break;
                case 'A' ... 'Z':
                    score += (fast_islower(*(tp + 1)) || !fast_isalpha(*(tp - 1)))
                        ? START_WORD_BONUS
                        : LETTER_MATCH_BONUS;
                    break;
                case '0' ... '9':
                    score += fast_isdigit(*(tp - 1))
                        ? LETTER_MATCH_BONUS
                        : START_WORD_BONUS;
                    break;
                default:
                    score += LETTER_MATCH_BONUS;
                    break;
            }

            score += isConsecutive * CONSECUTIVE_BONUS;
            qp++;
            if (!(*qp)) {
                break;
            }
            isConsecutive = true;
        }
        else {
            isConsecutive = false;
        }

        tp++;
    }

    if (*qp) {
        return 0;
    }
    return (!min_score || score < min_score) ? score : min_score;
}
