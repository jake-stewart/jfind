#ifndef ITEM_LIST_HPP
#define ITEM_LIST_HPP

#include "ansi_wrapper.hpp"
#include "config.hpp"
#include "item_cache.hpp"
#include "sliding_cache.hpp"
#include "style_manager.hpp"

class ItemList
{
    bool m_optimizeAnsi = false;

    int m_x = 0;
    int m_y = 0;
    int m_width = 0;
    int m_height = 0;

    int m_offset = 0;
    int m_cursor = 0;

    int m_nVisibleItems = 0;
    int m_itemWidth = 0;
    int m_hintWidth = 0;

    bool m_allowScrolling = true;

    FILE *m_outputFile;

    ItemCache *m_itemCache;

    AnsiWrapper &ansi = AnsiWrapper::instance();
    const Config &m_config = Config::instance();

    void drawName(int i) const;
    void drawHint(int i) const;
    void drawItems() const;
    void calcVisibleItems();

public:
    ItemList(ItemCache *itemCache);
    void canOptimizeAnsi(bool value);
    void allowScrolling(bool value);
    bool setSelected(int y);
    Item *getSelected() const;
    Item *get(int y) const;
    void resize(int x, int y, int w, int h);
    bool scrollUp();
    bool scrollDown();
    bool moveCursorUp();
    bool moveCursorDown();
    void refresh(bool resetCursor);

    float getScrollPercentage() const;
};

#endif
