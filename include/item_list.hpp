#ifndef ITEM_LIST_HPP
#define ITEM_LIST_HPP

#include "item_cache.hpp"
#include "style_manager.hpp"
#include "config.hpp"
#include "ansi_wrapper.hpp"
#include "sliding_cache.hpp"

class ItemList {
    int m_width = 0;
    int m_height = 0;

    int m_offset = 0;
    int m_cursor = 0;

    int m_nVisibleItems = 0;
    int m_itemWidth = 0;
    int m_hintWidth = 0;

    bool m_didScroll = false;
    bool m_allowScrolling = false;

    FILE *m_outputFile;

    ItemCache *m_itemCache;
    StyleManager *m_styleManager;

    Logger m_logger = Logger("ItemList");
    AnsiWrapper &ansi = AnsiWrapper::instance();
    Config& m_config = Config::instance();

    void drawName(int i) const;
    void drawHint(int i) const;
    void drawItems() const;
    void calcVisibleItems();

public:
    ItemList(FILE *outputFile, StyleManager *styleManager,
            ItemCache *itemCache);
    void allowScrolling(bool value);
    bool didScroll();
    bool setSelected(int y);
    Item* getSelected() const;
    Item* get(int y) const;
    void resize(int w, int h);
    void scrollUp();
    void scrollDown();
    void moveCursorUp();
    void moveCursorDown();
    void refresh(bool resetCursor);
};

#endif
