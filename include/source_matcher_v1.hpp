#include "letter_casing.hpp"

class SourceMatcherV1
{
private:
    bool valid;
    const char *query;
    const char *text;
    const char *tp;
    const char *qp;
    int  word;
    int  term;
    int  max_term;
    int  max_word;
    int  n_terms;
    int  match_length;

    LetterCasing casing;

    int matches;
    int max_consecutive_matches;

    void parseCasing();
    void nextWord();
    void nextTerm();
    bool isQueryWordEnd();
    void step();
    bool match();

public:
    int calc(const char *text, const char *query);
};
