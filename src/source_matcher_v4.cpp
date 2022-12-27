#include "../include/source_matcher_v4.hpp"
#include "../include/util.hpp"
#include <climits>

const int SCORE_MODIFIER = -100;
const int START_LINE_BONUS = 3 * SCORE_MODIFIER;
const int START_WORD_BONUS = 2 * SCORE_MODIFIER;
const int LETTER_MATCH_BONUS = 1 * SCORE_MODIFIER;
const int CONSECUTIVE_BONUS = 1 * SCORE_MODIFIER;

int SourceMatcherV4::calc(const char *text, const char **queries, int n_queries) {
    int total = 0;
    for (int i = 0; i < n_queries; i++) {
        int score = match_start(text, queries[i]);
        if (!score) {
            return INT_MAX;
        }
        total += score + strlen(text);
    }
    return total;
}

int SourceMatcherV4::match_start(const char *tp, const char *qp) {
    int min_score = 0;

    if (fast_tolower(*tp) == *qp) {
        if (*(qp + 1)) {
            int score = match(tp + 1, qp + 1);
            if (!score) {
                return min_score;
            }
            score += START_LINE_BONUS;
            min_score = score < min_score ? score : min_score;
        }
        else {
            return START_LINE_BONUS;
        }
    }

    int score = match(tp + 1, qp);
    if (score && score < min_score) {
        min_score = score;
    }

    return min_score;
}

int SourceMatcherV4::get_letter_score(const char *tp, const bool isConsecutive) {
    int accept_score;
    switch (*tp) {
        case 'a' ... 'z':
            accept_score = fast_isalpha(*(tp - 1))
                ? LETTER_MATCH_BONUS
                : START_WORD_BONUS;
            break;
        case 'A' ... 'Z':
            accept_score = (fast_islower(*(tp + 1)) || !fast_isalpha(*(tp - 1)))
                ? START_WORD_BONUS
                : LETTER_MATCH_BONUS;
            break;
        case '0' ... '9':
            accept_score = fast_isdigit(*(tp - 1))
                ? LETTER_MATCH_BONUS
                : START_WORD_BONUS;
            break;
        default:
            accept_score = LETTER_MATCH_BONUS;
            break;
    }

    return accept_score + isConsecutive * CONSECUTIVE_BONUS;
}

int SourceMatcherV4::match(const char *tp, const char *qp) {
    if (!(*qp)) {
        return 0;
    }

    bool isConsecutive = true;
    int min_score = 0;

    while (*tp) {
        if (fast_tolower(*tp) == *qp) {
            int score = match(tp + 1, qp + 1);
            if (!score) {
                if (!*(qp + 1)) {
                    score = get_letter_score(tp, isConsecutive);
                    if (score < min_score) {
                        min_score = score;
                    }
                }
                break;
            }
            score += get_letter_score(tp, isConsecutive);
            if (score < min_score) {
                min_score = score;
            }
        }

        isConsecutive = false;
        tp++;
    }

    return min_score;
}
