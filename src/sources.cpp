#include "../include/sources.hpp"
#include "../include/util.hpp"
#include "../include/source_matcher_v1.hpp"
#include "../include/source_matcher_v2.hpp"
#include "../include/source_matcher_v3.hpp"
#include <string.h>

void Sources::readWithHints(FILE *fp) {
    m_max_text_w = 0;
    size_t size;
    char *buf = nullptr;
    char *second_buf = nullptr;
    while (getline(&buf, &size, fp) >= 0) {
        getline(&second_buf, &size, fp);
        buf[strcspn(buf, "\n")] = 0;
        second_buf[strcspn(second_buf, "\n")] = 0;

        m_max_text_w = strlen(buf) > m_max_text_w ? strlen(buf) : m_max_text_w;

        char *third_buf = (char*)malloc(strlen(buf) + strlen(second_buf) + 2);

        strcpy(third_buf, buf);
        strcpy(third_buf + strlen(buf) + 1, second_buf);

        free(second_buf);
        free(buf);

        m_sources.push_back(Source(third_buf));

        buf = nullptr;
        second_buf = nullptr;
    }
}

void Sources::readWithoutHints(FILE *fp) {
    size_t size;
    char *buf = nullptr;
    while (getline(&buf, &size, fp) >= 0) {
        buf[strcspn(buf, "\n")] = 0;
        m_sources.push_back(Source(buf));
        buf = nullptr;
    }
}

void Sources::read(FILE *fp, bool hints) {
    if (hints) {
        readWithHints(fp);
    }
    else {
        readWithoutHints(fp);
    }
}

bool Sources::readFile(const char *fname, bool hints) {
    FILE *fp = fopen(fname, "r");
    if (fp == nullptr) {
        return false;
    }
    read(fp, hints);
    fclose(fp);
    return true;
}

Source Sources::get(int i) {
    return m_sources[i];
}

int Sources::size() {
    return m_sources.size();
}

bool compareSources(Source& l, Source& r) {
    if (l.heuristic == r.heuristic) {
        // if (strlen(l.text) == strlen(r.text)) {
        //     return strcmp(l.text, r.text) < 0;
        // }
        return strlen(l.text) < strlen(r.text);
    }
    return (l.heuristic < r.heuristic);
}

#include <thread>

#define N_WORKERS 8
SourceMatcherV3 matchers[N_WORKERS];

void thread_func(Source *source, Source *end, bool skip_empty, int matcher, const char **words, int n_words) {
    while (source != end) {
        if (skip_empty && source->heuristic == INT_MAX) {
            source++;
            continue;
        }
        (*source).heuristic = matchers[matcher].calc((*source).text, words, n_words);
        source++;
    }
}

void Sources::sort(const char* query, bool skip_empty) {
    std::vector<std::thread> threads;


    char *s = new char[strlen(query) + 1];
    const char **words = new const char*[10];
    strcpy(s, query);
    // loop through the string to extract all other tokens
    int n_words = 0;
    char* token = strtok(s, " ");
    // loop through the string to extract all other tokens
    while (token != NULL) {
        words[n_words++] = token;
        if (n_words == 10) {
            break;
        }
        token = strtok(NULL, " ");
    }
    // n_words = 0;

    int portion = m_sources.size() / N_WORKERS;

    if (N_WORKERS > 1 && m_sources.size() > 10000) {
        std::vector<std::thread> threads(N_WORKERS - 1);

        int portion = m_sources.size() / N_WORKERS;

        int i;
        for (i = 0; i < N_WORKERS - 1; i++) {
            threads[i] = (
                std::thread(
                    thread_func,
                    m_sources.data() + (portion * i),
                    m_sources.data() + (portion * (i + 1)),
                    skip_empty,
                    i,
                    words,
                    n_words
                )
            );
        }

        thread_func(
            m_sources.data() + (portion * i),
            m_sources.data() + m_sources.size(),
            skip_empty,
            i,
            words,
            n_words
        );

        for (i = 0; i < N_WORKERS - 1; i++) {
            threads[i].join();
        }
    }
    else {
        for (Source& source : m_sources) {
            if (skip_empty && source.heuristic == INT_MAX) {
                continue;
            }
            source.heuristic = matchers[0].calc(source.text, words, n_words);
        }
    }

    delete[] s;
    delete[] words;

    std::sort(m_sources.begin(), m_sources.end(), compareSources);
}

int Sources::getMaxWidth() {
    return m_max_text_w;
}
