#ifndef EVENT_HPP
#define EVENT_HPP

#include "item.hpp"
#include "key.hpp"
#include "mouse_event.hpp"
#include "item_preview_content.hpp"
#include <cstring>
#include <vector>

enum EventType {
    KEY_EVENT,
    QUERY_CHANGE_EVENT,
    NEW_ITEMS_EVENT,
    ALL_ITEMS_READ_EVENT,
    ITEMS_ADDED_EVENT,
    ITEMS_SORTED_EVENT,
    ITEMS_REQUEST_EVENT,
    RESIZE_EVENT,
    PREVIEW_READ_EVENT,
    SELECTED_ITEM_CHANGE_EVENT,
};
const char *const *getEventNames();

class Event
{
public:
    virtual ~Event() {}
    virtual EventType getType() const = 0;
};

class KeyEvent : public Event
{
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

    EventType getType() const override {
        return KEY_EVENT;
    }

    Key getKey() {
        return m_key;
    }

    char *getWidechar() {
        return m_widechar;
    }

    std::vector<MouseEvent> &getMouseEvents() {
        return m_mouseEvents;
    }
};

class QueryChangeEvent : public Event
{
    std::string m_query;

public:
    QueryChangeEvent(std::string query) {
        m_query = query;
    }

    EventType getType() const override {
        return QUERY_CHANGE_EVENT;
    }

    const std::string &getQuery() {
        return m_query;
    }
};

class NewItemsEvent : public Event
{
    std::vector<Item> *m_items;

public:
    NewItemsEvent(std::vector<Item> *items) {
        m_items = items;
    }

    EventType getType() const override {
        return NEW_ITEMS_EVENT;
    }

    std::vector<Item> *getItems() {
        return m_items;
    }
};

class AllItemsReadEvent : public Event
{
    bool m_value;

public:
    AllItemsReadEvent(bool value) {
        m_value = value;
    }

    EventType getType() const override {
        return ALL_ITEMS_READ_EVENT;
    }

    bool getValue() {
        return m_value;
    }
};

class ItemsAddedEvent : public Event
{
public:
    EventType getType() const override {
        return ITEMS_ADDED_EVENT;
    }
};

class ItemsSortedEvent : public Event
{
    std::string m_query;

public:
    ItemsSortedEvent(std::string query) {
        m_query = query;
    }

    const std::string &getQuery() {
        return m_query;
    }

    EventType getType() const override {
        return ITEMS_SORTED_EVENT;
    }
};

class ResizeEvent : public Event
{
    int m_width;
    int m_height;

public:
    ResizeEvent(int width, int height) {
        m_width = width;
        m_height = height;
    }

    EventType getType() const override {
        return RESIZE_EVENT;
    }

    int getWidth() const {
        return m_width;
    }

    int getHeight() const {
        return m_height;
    }
};

class ItemsRequestEvent : public Event
{
    EventType getType() const override {
        return ITEMS_REQUEST_EVENT;
    }
};

class PreviewReadEvent : public Event
{
    ItemPreviewContent m_content;

public:
    PreviewReadEvent(ItemPreviewContent content) {
        m_content = content;
    }

    EventType getType() const override {
        return PREVIEW_READ_EVENT;
    }

    ItemPreviewContent getContent() const {
        return m_content;
    }

};

class SelectedItemChangeEvent : public Event
{
    std::string m_item;

public:
    SelectedItemChangeEvent(std::string item) {
        m_item = item;
    }

    EventType getType() const override {
        return SELECTED_ITEM_CHANGE_EVENT;
    }

    std::string getItem() const {
        return m_item;
    }
};

#endif
