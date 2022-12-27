#include "letter_casing.hpp"

class SourceMatcherV4
{
public:
    int calc(const char *text, const char **queries, int n_queries);

private:
    int match_start(const char *tp, const char *qp);
    int match(const char *tp, const char *qp);
    int get_letter_score(const char *tp, const bool isConsecutive);
};
