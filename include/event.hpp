#ifndef EVENT_HPP
#define EVENT_HPP

#include "mouse_event.hpp"
#include "key.hpp"
#include "item.hpp"
#include <vector>

enum EventType {
    KEY_EVENT,
    QUERY_CHANGE_EVENT,
    NEW_ITEMS_EVENT,
    ALL_ITEMS_READ_EVENT,
    ITEMS_ADDED_EVENT,
    ITEMS_SORTED_EVENT,
    RESIZE_EVENT,
    QUIT_EVENT,
};
const char** getEventNames();

class Event {
public:
    virtual ~Event() {}
    virtual EventType getType() = 0;
};

class KeyEvent : public Event {
    Key m_key;
    std::vector<MouseEvent> m_mouseEvents;
    char m_widechar[4];

public:
    KeyEvent(Key key) {
        m_key = key;
    }

    KeyEvent(Key key, char widechar[4]) {
        m_key = key;
        memcpy(m_widechar, widechar, 4);
    }

    KeyEvent(Key key, std::vector<MouseEvent> mouseEvents) {
        m_key = key;
        m_mouseEvents = mouseEvents;
    }

    EventType getType() {
        return KEY_EVENT;
    }

    Key getKey() {
        return m_key;
    }

    char* getWidechar() {
        return m_widechar;
    }

    std::vector<MouseEvent>& getMouseEvents() {
        return m_mouseEvents;
    }
};

class QueryChangeEvent : public Event {
    std::string m_query;

public:
    QueryChangeEvent(std::string query) {
        m_query = query;
    }

    EventType getType() {
        return QUERY_CHANGE_EVENT;
    }

    std::string getQuery() {
        return m_query;
    }
};

class NewItemsEvent : public Event {
    std::vector<Item>* m_items;

public:
    NewItemsEvent(std::vector<Item>* items) {
        m_items = items;
    }

    EventType getType() {
        return NEW_ITEMS_EVENT;
    }

    std::vector<Item>* getItems() {
        return m_items;
    }
};

class AllItemsReadEvent : public Event {
    EventType getType() {
        return ALL_ITEMS_READ_EVENT;
    }
};

class QuitEvent : public Event {
    EventType getType() {
        return QUIT_EVENT;
    }
};

class ItemsAddedEvent : public Event {
    EventType getType() {
        return ITEMS_ADDED_EVENT;
    }
};

class ItemsSortedEvent : public Event {
    EventType getType() {
        return ITEMS_SORTED_EVENT;
    }
};

class ResizeEvent : public Event {
    int m_width;
    int m_height;

public:
    ResizeEvent(int width, int height) {
        m_width = width;
        m_height = height;
    }

    EventType getType() {
        return RESIZE_EVENT;
    }

    int getWidth() {
        return m_width;
    }

    int getHeight() {
        return m_height;
    }
};

#endif
