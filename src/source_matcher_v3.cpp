#include "../include/source_matcher_v3.hpp"
#include "../include/util.hpp"

#include <climits>

struct ScoreModifier {
    static const int history = 1;
    static const int consecutive = 2;
    static const int boundary = 3;
    static const int distance = 1;
};

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

int SourceMatcherV3::match(const char *tp, const char *qp, bool is_start) {
    int score = strlen(tp) / 2;

    while (*tp && *qp) {
        if (tolower(*tp) == *qp) {
            if (is_start) {
                score -= 3;
            }
            else if (islower(*tp)) {
                if (!isalpha(*(tp - 1))) {
                    score -= 2;
                }
                else {
                    score -= 1;
                }
            }
            else if (isupper(*tp)) {
                if (islower(*(tp + 1))) {
                    score -= 2;
                }
                else if (!isalpha(*(tp - 1))) {
                    score -= 2;
                }
                else {
                    score -= 1;
                }
            }
            else if (isnumber(*tp)) {
                if (!isnumber(*(tp - 1))) {
                    score -= 2;
                }
                else {
                    score -= 1;
                }
            }
            else {
                score -= 1;
            }
            qp++;
        }

        is_start = false;
        tp++;
    }

    return *qp ? 0 : score;
}

LetterCasing SourceMatcherV3::parseCase(const char *tp) {
    if (islower(*tp)) {
        return LOWER_CASE;
    }
    else if (isdigit(*tp)) {
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

int SourceMatcherV3::matchWord(const char **tpp, const char **qpp) {
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
            while (isdigit(*tp) && *tp == *qp) {
                i++; tp++; qp++;
            }
            while (isdigit(*tp)) {
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
