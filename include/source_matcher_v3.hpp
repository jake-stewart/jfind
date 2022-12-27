#include "letter_casing.hpp"

class SourceMatcherV3
{
public:
    int calc(const char *text, const char **queries, int n_queries);

private:
    int match(const char *tp, const char *qp, bool is_start);
    int match_recurse(const char *tp, const char *qp, bool is_start);
};
