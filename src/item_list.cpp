#include "../include/item_list.hpp"
#include "../include/logger.hpp"
#include "../include/utf8_string.hpp"
#include <cstring>

ItemList::ItemList(ItemCache *itemCache) {
    m_itemCache = itemCache;
}

void ItemList::canOptimizeAnsi(bool value) {
    m_optimizeAnsi = value;
}

void ItemList::drawItems() const {
    if (m_width <= 2 || m_height <= 0) {
        return;
    }

    for (int i = m_offset; i < m_offset + m_nVisibleItems; i++) {
        drawName(i);
    }

    if (m_hintWidth >= m_config.minHintWidth) {
        for (int i = m_offset; i < m_nVisibleItems + m_offset; i++) {
            drawHint(i);
        }
    }

    for (int i = m_nVisibleItems; i < m_height; i++) {
        StyleManager::instance().set(m_config.backgroundStyle);
        if (Config::instance().queryPlacement == VerticalPlacement::Top) {
            ansi.move(m_x, m_y + i);
        }
        else {
            ansi.move(m_x, m_y + m_height - i - 1);
        }
        if (m_optimizeAnsi) {
            ansi.clearTilEOL();
        }
        else {
            for (int i = 0; i < m_width; i++) {
                fprintf(stderr, " ");
            }
        }
    }
}

void ItemList::drawName(int i) const {
    std::string name = std::string(m_itemCache->get(i)->text);
    replace(name, '\t', ' ');
    // for (int i = 0; i < name.size(); i++) {
    //     if (name[i] < 32) {
    //         name[i] = '?';
    //     }
    // }
    Utf8String string;
    Utf8StringCursor cursor;
    cursor.setString(&string);
    cursor.insert(name);
    cursor.reset();
    int bytes = cursor.getBytesForCols(m_itemWidth - 1);

    // if (name.size() > m_itemWidth) {
    //     name = name.substr(0, m_itemWidth - 1) + "…";
    // }

    if (Config::instance().queryPlacement == VerticalPlacement::Top) {
        ansi.move(m_x, m_y + i - m_offset);
    }
    else {
        ansi.move(m_x, m_y + m_height - i - 1 + m_offset);
    }
    if (i == m_cursor) {
        if (m_config.activeSelector.size()) {
            StyleManager::instance().set(m_config.activeSelectorStyle);
            fprintf(stderr, "%s", m_config.activeSelector.c_str());
        }
        StyleManager::instance().set(m_config.activeItemStyle);
    }
    else {
        if (m_config.selector.size()) {
            StyleManager::instance().set(m_config.selectorStyle);
            fprintf(stderr, "%s", m_config.selector.c_str());
        }
        StyleManager::instance().set(m_config.itemStyle);
    }

    // fprintf(stderr, "%s", name.c_str());
    fprintf(stderr, "%.*s", bytes, name.c_str());

    StyleManager::instance().set(
        i == m_cursor ? m_config.activeRowStyle : m_config.rowStyle
    );
    if (m_optimizeAnsi) {
        ansi.clearTilEOL();
    }
    else {
        for (int i = name.size(); i < m_width; i++) {
            fprintf(stderr, " ");
        }
    }
}

void ItemList::drawHint(int i) const {
    const char *text = m_itemCache->get(i)->text;
    std::string hint = std::string(text + strlen(text) + 1);
    replace(hint, '\t', ' ');
    for (int i = 0; i < hint.size(); i++) {
        if (hint[i] < 32) {
            hint[i] = '?';
        }
    }

    StyleManager::instance().set(
        i == m_cursor ? m_config.activeHintStyle : m_config.hintStyle
    );

    if (hint.size() > m_hintWidth) {
        const char *str = hint.data();
        int startIdx = hint.size() - m_hintWidth + 1;
        int idx = startIdx;
        while (str[idx] != '/') {
            idx++;
            if (idx == hint.size()) {
                idx = startIdx;
                break;
            }
        }
        if (Config::instance().queryPlacement == VerticalPlacement::Top) {
            ansi.move(
                m_x + m_width - hint.size() + idx - 1,
                m_y + i - m_offset
            );
        }
        else {
            ansi.move(
                m_x + m_width - hint.size() + idx - 1,
                m_y + m_height - i - 1 + m_offset
            );
        }
        fprintf(stderr, "…");
        fprintf(stderr, "%s", str + idx);
    }
    else {
        if (Config::instance().queryPlacement == VerticalPlacement::Top) {
            ansi.move(
                m_x + m_width - hint.size(), m_y + i - m_offset
            );
        }
        else {
            ansi.move(
                m_x + m_width - hint.size(), m_y + m_height - i - 1 + m_offset
            );
        }
        fprintf(stderr, "%s", hint.data());
    }
}

bool ItemList::scrollUp() {
    if (!m_allowScrolling) {
        return false;
    }
    Item *item = m_itemCache->get(m_offset + m_height);
    if (item == nullptr || item->heuristic == BAD_HEURISTIC) {
        return false;
    }
    m_offset += 1;
    if (!m_optimizeAnsi || m_height <= 1) {
        if (m_cursor - m_offset < 0) {
            m_cursor += 1;
        }
        drawItems();
        return true;
    }
    ansi.setScrollRegion(m_y, m_y + m_height - 1);
    if (Config::instance().queryPlacement == VerticalPlacement::Top) {
        ansi.scrollUp();
    }
    else {
        ansi.scrollDown();
    }
    if (m_cursor - m_offset < 0) {
        m_cursor += 1;
        drawName(m_offset);
        if (m_hintWidth > m_config.minHintWidth) {
            drawHint(m_offset);
        }
    }
    drawName(m_offset + m_height - 1);
    if (m_hintWidth > m_config.minHintWidth) {
        drawHint(m_offset + m_height - 1);
    }
    return true;
}

bool ItemList::scrollDown() {
    if (!m_allowScrolling || m_offset <= 0) {
        return false;
    }
    m_offset -= 1;
    if (!m_optimizeAnsi || m_height <= 1) {
        if (m_cursor - m_offset >= m_height) {
            m_cursor -= 1;
        }
        drawItems();
        return true;
    }
    ansi.setScrollRegion(m_y, m_y + m_height - 1);
    if (Config::instance().queryPlacement == VerticalPlacement::Top) {
        ansi.scrollDown();
    }
    else {
        ansi.scrollUp();
    }
    if (m_cursor - m_offset >= m_height) {
        m_cursor -= 1;
        drawName(m_offset + m_height - 1);
        if (m_hintWidth > m_config.minHintWidth) {
            drawHint(m_offset + m_height - 1);
        }
    }
    drawName(m_offset);
    if (m_hintWidth > m_config.minHintWidth) {
        drawHint(m_offset);
    }
    return true;
}

bool ItemList::moveCursorUp() {
    Item *item = m_itemCache->get(m_cursor + 1);
    if (item == nullptr || item->heuristic == BAD_HEURISTIC) {
        if (!m_allowWrapping) {
            return false;
        }
        m_cursor = 0;
        m_offset = 0;
        drawItems();
        return true;
    }

    m_cursor += 1;
    if (m_cursor - m_offset >= m_nVisibleItems) {
        if (!m_allowScrolling) {
            m_cursor -= 1;
            return false;
        }
        m_offset += 1;
        if (!m_optimizeAnsi || m_height <= 1) {
            drawItems();
            return true;
        }
        ansi.setScrollRegion(m_y, m_y + m_height - 1);
        if (Config::instance().queryPlacement == VerticalPlacement::Top) {
            ansi.scrollUp();
        }
        else {
            ansi.scrollDown();
        }
    }
    drawName(m_cursor - 1);
    drawName(m_cursor);
    if (m_hintWidth > m_config.minHintWidth) {
        drawHint(m_cursor - 1);
        drawHint(m_cursor);
    }
    return true;
}

bool ItemList::moveCursorDown() {
    if (m_cursor <= 0) {
        if (!m_allowScrolling) {
            return false;
        }
        if (!m_allowWrapping) {
            return false;
        }
        m_cursor = m_itemCache->size() - 1;
        m_offset = m_itemCache->size() - m_height;
        if (m_offset < 0) {
            m_offset = 0;
        }
        drawItems();
        return true;
    }
    m_cursor -= 1;
    if (m_cursor - m_offset < 0) {
        if (!m_allowScrolling) {
            m_cursor += 1;
            return false;
        }
        m_offset -= 1;
        if (!m_optimizeAnsi || m_height <= 1) {
            drawItems();
            return true;
        }
        ansi.setScrollRegion(m_y, m_y + m_height - 1);
        if (Config::instance().queryPlacement == VerticalPlacement::Top) {
            ansi.scrollDown();
        }
        else {
            ansi.scrollUp();
        }
    }
    drawName(m_cursor + 1);
    drawName(m_cursor);
    if (m_hintWidth > m_config.minHintWidth) {
        drawHint(m_cursor + 1);
        drawHint(m_cursor);
    }
    return true;
}

void ItemList::calcVisibleItems() {
    int size = m_itemCache->size();
    m_nVisibleItems = size > m_height ? m_height : size;

    for (int i = 0; i < m_nVisibleItems; i++) {
        Item *item = m_itemCache->get(m_offset + i);
        if (item == nullptr || item->heuristic == BAD_HEURISTIC) {
            m_nVisibleItems = i;
            break;
        }
    }
}

void ItemList::resize(int x, int y, int w, int h) {
    bool firstResize = m_width == 0;

    m_x = x;
    m_y = y;
    m_width = w;
    m_height = h;

    if (h > m_itemCache->getReserve() * 2) {
        m_itemCache->setReserve(h * 2);
    }

    int selectorWidth = std::max(
        m_config.selector.size(), m_config.activeSelector.size()
    );

    if (m_config.showHints) {
        m_hintWidth =
            (m_width - ((m_width / 5) * 3) - selectorWidth -
             m_config.minHintSpacing);
        if (m_hintWidth >= m_config.minHintWidth) {
            if (m_hintWidth >= m_config.maxHintWidth) {
                m_hintWidth = m_config.maxHintWidth;
            }
            m_itemWidth = m_width - m_hintWidth - selectorWidth -
                m_config.minHintSpacing;
        }
        else {
            m_hintWidth = 0;
            m_itemWidth = m_width - m_hintWidth - selectorWidth;
        }
    }
    else {
        m_hintWidth = 0;
        m_itemWidth = m_width - m_hintWidth - selectorWidth;
    }

    if (!firstResize) {
        if (m_height + m_offset > m_itemCache->size()) {
            m_offset = m_itemCache->size() - m_height;
            if (m_offset < 0) {
                m_offset = 0;
            }
        }
        calcVisibleItems();
        if (m_cursor - m_offset < 0) {
            m_offset = m_cursor;
        }
        if (m_cursor - m_offset >= m_nVisibleItems) {
            m_offset = m_cursor - m_nVisibleItems + 1;
        }
        drawItems();
    }

}

bool ItemList::setSelected(int y) {
    int idx;
    if (Config::instance().queryPlacement == VerticalPlacement::Top) {
        idx = y - m_y;
    }
    else {
        idx = m_y + m_height - y - 1;
    }
    if (idx >= m_nVisibleItems) {
        return false;
    }
    int oldCursor = m_cursor;
    m_cursor = m_offset + idx;
    drawName(oldCursor);
    drawName(m_cursor);
    if (m_hintWidth > m_config.minHintWidth) {
        drawHint(oldCursor);
        drawHint(m_cursor);
    }
    return true;
}

Item *ItemList::get(int y) const {
    if (Config::instance().queryPlacement == VerticalPlacement::Top) {
        return m_itemCache->get(m_offset + y - m_y);
    }
    else {
        return m_itemCache->get(m_offset + m_y + m_height - y - 1);
    }
}

void ItemList::allowScrolling(bool value) {
    m_allowScrolling = value;
}

void ItemList::allowWrapping(bool value) {
    m_allowWrapping = value;
}

void ItemList::refresh(bool resetCursor) {
    if (m_nVisibleItems < 0) {
        return;
    }
    bool visibleItemsChanged = false;
    if (resetCursor) {
        m_cursor = 0;
        if (m_offset != 0) {
            visibleItemsChanged = true;
        }
        m_offset = 0;
    }

    std::vector<int> itemIds(m_nVisibleItems);
    for (int i = 0; i < m_nVisibleItems; i++) {
        itemIds[i] = m_itemCache->get(m_offset + i)->index;
    }
    m_itemCache->refresh();
    calcVisibleItems();
    visibleItemsChanged = visibleItemsChanged ||
        m_nVisibleItems != itemIds.size();

    if (!visibleItemsChanged) {
        for (int i = 0; i < m_nVisibleItems; i++) {
            Item *item = m_itemCache->get(m_offset + i);
            if (itemIds[i] != item->index) {
                visibleItemsChanged = true;
                break;
            }
        }
    }

    if (visibleItemsChanged) {
        calcVisibleItems();
        drawItems();
    }
}

Item *ItemList::getSelected() const {
    if (m_nVisibleItems) {
        return m_itemCache->get(m_cursor);
    }
    return nullptr;
}

float ItemList::getScrollPercentage() const {
    return (float)(m_offset + m_nVisibleItems) / m_itemCache->size();
}
