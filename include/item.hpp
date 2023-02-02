#ifndef ITEM_HPP
#define ITEM_HPP

#include <climits>

const int BAD_HEURISTIC = -INT_MAX;

// an item represents a single record being queried
// it is 16 bytes (text (8) + heuristic (4) + index (4))

struct Item {
    // text: what the user searches for
    // the text may contain a hint after its null terminator
    // this saves having to store another char pointer (8 bytes each)
    char *text;

    // heuristic: the score of the item based on the current query
    // the heuristic is -INT_MAX if the text fails to match
    int heuristic;

    // index: the position of the item when it was read from stdin
    // when there is no query, items are sorted based on their index
    // the index can be negative to push the item to the front
    // eg. if --history is used, recent items will have a negative index
    int index;
};

#endif
