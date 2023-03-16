#ifndef ITEM_MATCHER_HPP
#define ITEM_MATCHER_HPP

#include <vector>
#include <string>

class ItemMatcher {
    public:
        int calc(const char *text, std::vector<std::string>& queries);

    private:
        int matchStart(const char *tp, const char *qp);
        int match(const char *tp, const char *qp, int distance,
                bool consecutive, int *depth);
};

#endif
