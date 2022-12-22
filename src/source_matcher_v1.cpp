#include "../include/source_matcher_v1.hpp"
#include "../include/util.hpp"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <climits>


void SourceMatcherV1::parseCasing() {
    if (is_upper(*tp)) {
        if (is_lower(*(tp + 1))) {
            casing = CAMEL_CASE;
        }
        else {
            casing = UPPER_CASE;
        }
    }
    else if (is_number(*tp)) {
        casing = NUMBER_CASE;
    }
    else {
        casing = LOWER_CASE;
    }
}

void SourceMatcherV1::nextWord() {
    switch (casing) {
        case CAMEL_CASE:
            if (is_upper(*tp)) {
                tp++;
            }
            while (is_lower(*tp)) {
                tp++;
            }
            break;

        case UPPER_CASE:
            while (is_upper(*tp) && !is_lower(*(tp + 1))) {
                tp++;
            }
            break;

        case NUMBER_CASE:
            while (is_number(*tp)) {
                tp++;
            }
            break;

        case LOWER_CASE:
            while (is_lower(*tp)) {
                tp++;
            }
            break;
        default:
            break;
    }

    while (*tp && !is_alnum(*tp)) {
        tp++;
    }

    parseCasing();
}

void SourceMatcherV1::nextTerm() {
    while (is_lower(*qp) || is_number(*qp)) {
        qp++;
    }
    while (*qp && !is_lower_alnum(*qp)) {
        qp++;
    }
}

bool SourceMatcherV1::isQueryWordEnd() {
    return !*qp || *qp == ' ';
}

void SourceMatcherV1::step() {
    tp++;
    qp++;
    match_length++;
}

bool SourceMatcherV1::match() {
    parseCasing();

    match_length = 0;

    switch (casing) {
        case LOWER_CASE:
            while (is_lower(*tp)) {
                if (*tp != *qp) {
                    return isQueryWordEnd();
                }
                step();
            }
            return isQueryWordEnd();

        case NUMBER_CASE:
            while (is_number(*tp)) {
                if (*tp != *qp) {
                    return isQueryWordEnd();
                }
                step();
            }
            return isQueryWordEnd();

        case CAMEL_CASE:
            if (tolower(*tp) != *qp) {
                return false;
            }
            step();
            while (is_lower(*tp)) {
                casing = LOWER_CASE;
                if (*tp != *qp) {
                    return isQueryWordEnd();
                }
                step();
            }
            return isQueryWordEnd();

        case UPPER_CASE:
            while (is_upper(*(tp + 1))) {
                if (tolower(*tp) != *qp) {
                    return isQueryWordEnd();
                }
                step();
            }
            if (isQueryWordEnd()) {
                return true;
            }
            if (is_upper(*tp)) {
                if (tolower(*tp) != *qp) {
                    return false;
                }
                step();
            }
            return isQueryWordEnd();

        default:
            return false;
    }
}

int SourceMatcherV1::calc(const char *text, const char *query) {
    int heuristic = 0;

    // for (int i = 0; i < strlen(query); i++) {
    //     if (!strchr(text, query[i])) {
    //         return -INT_MAX;
    //     }
    // }

    this->query = query;
    this->text = text;
    this->tp = text;
    this->qp = query;

    term = 0;
    max_term = 0;
    max_word = -1;
    n_terms = 0;
    valid = false;
    word = 0;
    max_consecutive_matches = 0;

    parseCasing();

    int query_heuristics[10] = {0};

    // for word in text
    while (*tp) {
        term = 0;
        qp = query;

        // save current casing/text cursor
        // so we can return to the start of the word
        // and compare it against every search term
        LetterCasing text_start_casing = casing;
        const char *text_start = tp;

        // for term in query
        while (*qp) {

            // goto start of word
            casing = text_start_casing;
            tp = text_start;

            if (match()) {

                if (match_length > query_heuristics[term]) {
                    query_heuristics[term] = match_length;
                }
                // heuristic += match_length * 10;

                const char *query_start = qp;

                if (term == max_term && word > max_word) {
                    max_word = word;
                    ++max_term;
                }

                nextTerm();
                nextWord();

                int consecutive = 1;
                int length = 0;

                while (*tp && *qp) {
                    if (!match()) {
                        break;
                    }
                    nextTerm();
                    nextWord();
                    consecutive++;
                    length += match_length;

                    int h = length * consecutive;
                    if (h > query_heuristics[term]) {
                        query_heuristics[term] = h;
                    }
                    // heuristic += match_length * 200 * consecutive;
                }

                if (consecutive > max_consecutive_matches) {
                    max_consecutive_matches = consecutive;
                }

                qp = query_start;
            }

            if (++term > n_terms) {
                n_terms = term;
            }
            nextTerm();
        }

        casing = text_start_casing;
        tp = text_start;
        nextWord();
        word++;
    }

    valid = max_term >= n_terms;
    if (!valid) {
        return -INT_MAX;
    }

    for (int i = 0; i < 10; i++) {
        heuristic += query_heuristics[i];
    }
    return heuristic;
}
