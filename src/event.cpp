#include "../include/event.hpp"

static const char* const EVENT_NAMES[] = {
    "KEY_EVENT",
    "QUERY_CHANGE_EVENT",
    "NEW_ITEMS_EVENT",
    "ALL_ITEMS_READ_EVENT",
    "ITEMS_ADDED_EVENT",
    "ITEMS_SORTED_EVENT",
    "RESIZE_EVENT",
    "QUIT_EVENT",
};

const char* const* getEventNames() {
    return EVENT_NAMES;
}
