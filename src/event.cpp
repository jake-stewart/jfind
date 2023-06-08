#include "../include/event.hpp"

static const char *EVENT_NAMES[] = {
    "KEY_EVENT",
    "QUERY_CHANGE_EVENT",
    "NEW_ITEMS_EVENT",
    "ALL_ITEMS_READ_EVENT",
    "ITEMS_ADDED_EVENT",
    "ITEMS_SORTED_EVENT",
    "RESIZE_EVENT",
    "QUIT_EVENT",
};

const char **getEventNames() {
    return EVENT_NAMES;
}
