#include "../include/source_matcher_v2.hpp"
#include "../include/util.hpp"

#include <stdlib.h>

int SourceMatcherV2::calc(const char *text, const char **queries, int n_queries) {
    int total = 0;
    for (int i = 0; i < n_queries; i++) {
        if (!fuzzy(text, queries[i])) {
            return INT_MAX;
        }
        int score = match(text, queries[i], true);
        if (!score) {
            score = 1000;
        }
        total += score;
    }
    return total;
}

bool SourceMatcherV2::fuzzy(const char *tp, const char *qp) {
    while (*tp && *qp) {
        if (tolower(*tp) == *qp) {
            qp++;
        }
        tp++;
    }
    return !*qp;
}

int SourceMatcherV2::match(const char *tp, const char *qp, bool is_start) {
    const char *start = tp;
    int n_matched_chars = 0;
    int min_score = 0;

    while (*tp && *qp) {
        if (!is_alnum(*tp)) {
            tp++;
            continue;
        }
        if (is_start) {
            start = tp;
        }
        n_matched_chars = matchWord(&tp, &qp);
        for (int i = 1; i < n_matched_chars + 1; i++) {
            bool next_start = is_start && i == n_matched_chars;
            int score = match(tp, qp - i, next_start);

            if (score) {
                if (!next_start) {
                    score += (tp - start);
                }
                min_score = !min_score || score < min_score
                    ? score
                    : min_score;
            }
        }
        if (n_matched_chars) {
            is_start = false;
        }
    }

    if (*qp) {
        return min_score;
    }
    int score = tp - start;
    if (n_matched_chars < score) {
        score -= n_matched_chars;
    }
    return !min_score || score < min_score ? score : min_score;
}

LetterCasing SourceMatcherV2::parseCase(const char *tp) {
    if (islower(*tp)) {
        return LOWER_CASE;
    }
    else if (isnumber(*tp)) {
        return NUMBER_CASE;
    }
    else if (isupper(*tp)) {
        if (islower(*(tp + 1))) {
            return CAMEL_CASE;
        }
        else {
            return UPPER_CASE;
        }
    }
    else {
        return NO_CASE;
    }
}

int SourceMatcherV2::matchWord(const char **tpp, const char **qpp) {
    int i = 0;
    const char *tp = *tpp;
    const char *qp = *qpp;

    switch (parseCase(tp)) {
        case LOWER_CASE:
            while (islower(*tp) && *tp == *qp) {
                i++; tp++; qp++;
            }
            while (*tp && islower(*tp)) {
                tp++;
            }
            break;

        case UPPER_CASE:
            while (
                    isupper(*tp)
                    && !islower(*(tp + 1))
                    && tolower(*tp) == *qp
            ) {
                i++; tp++; qp++;
            }
            while (isupper(*tp) && !islower(*(tp + 1))) {
                tp++;
            }
            break;

        case CAMEL_CASE:
            if (tolower(*tp) == *qp) {
                i++; tp++; qp++;
                while (islower(*tp) && *tp == *qp) {
                    i++; tp++; qp++;
                }
            }
            else {
                tp++;
            }
            while (islower(*tp)) {
                tp++;
            }
            break;

        case NUMBER_CASE:
            while (isnumber(*tp) && *tp == *qp) {
                i++; tp++; qp++;
            }
            while (isnumber(*tp)) {
                tp++;
            }
            break;

        default:
            exit(1);
    }

    *qpp = qp;
    *tpp = tp;
    return i;
}
