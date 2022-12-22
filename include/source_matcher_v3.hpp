#include "letter_casing.hpp"

class SourceMatcherV3
{
public:
    int calc(const char *text, const char **queries, int n_queries);

private:
    char queryChar();
    LetterCasing parseCase(const char *tp);
    int match(const char *tp, const char *qp, bool is_start);
    int matchWord(const char **tpp, const char **qpp);
    bool fuzzy(const char *tp, const char *qp);
};
