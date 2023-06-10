#include "../include/item_list.hpp"

ItemList::ItemList(FILE *outputFile, StyleManager *styleManager,
            ItemCache *itemCache) {
    m_outputFile = outputFile;
    m_styleManager = styleManager;
    m_itemCache = itemCache;
}

void ItemList::drawItems() const {
    if (m_width <= 2 || m_height <= 1) {
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
    if (m_height - m_nVisibleItems - 2 >= 0) {
        ansi.move(m_width - 1, m_height - m_nVisibleItems - 2);
        m_styleManager->set(m_config.backgroundStyle);
        ansi.clearTilSOF();
    }
}

void ItemList::drawName(int i) const {
    std::string name = std::string(m_itemCache->get(i)->text);

    if (name.size() > m_itemWidth) {
        name = name.substr(0, m_itemWidth - 1) + "…";
    }

    ansi.move(0, m_height - i - 2 + m_offset);
    if (i == m_cursor) {
        m_styleManager->set(m_config.activeRowStyle);
        ansi.clearTilEOL();
        if (m_config.activeSelector.size()) {
            m_styleManager->set(m_config.activeSelectorStyle);
            fprintf(m_outputFile, "%s", m_config.activeSelector.c_str());
        }
        m_styleManager->set(m_config.activeItemStyle);
    }
    else {
        m_styleManager->set(m_config.rowStyle);
        ansi.clearTilEOL();
        if (m_config.selector.size()) {
            m_styleManager->set(m_config.selectorStyle);
            fprintf(m_outputFile, "%s", m_config.selector.c_str());
        }
        m_styleManager->set(m_config.itemStyle);
    }

    fprintf(m_outputFile, "%s", name.c_str());
    // fprintf(m_outputFile, "%s %d", name.c_str(), m_itemCache->get(i)->heuristic);
}

void ItemList::drawHint(int i) const {
    char *text = m_itemCache->get(i)->text;
    std::string hint = std::string(text + strlen(text) + 1);

    m_styleManager->set(i == m_cursor ? m_config.activeHintStyle
            : m_config.hintStyle);

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
        ansi.move(m_width - hint.size() + idx - 1, m_height - i - 2 + m_offset);
        fprintf(m_outputFile, "…");
        fprintf(m_outputFile, "%s", str + idx);
    }
    else {
        ansi.move(m_width - hint.size(), m_height - i - 2 + m_offset);
        fprintf(m_outputFile, "%s", hint.data());
    }
}

void ItemList::scrollUp() {
    if (!m_allowScrolling) {
        return;
    }
    Item *item = m_itemCache->get(m_offset + m_height - 1);
    if (item == nullptr || item->heuristic == BAD_HEURISTIC) {
        return;
    }

    m_offset += 1;
    ansi.moveHome();
    ansi.moveUpOrScroll();
    if (m_cursor - m_offset < 0) {
        m_cursor += 1;
        drawName(m_offset);
        if (m_hintWidth > m_config.minHintWidth) {
            drawHint(m_offset);
        }
    }

    drawName(m_offset + m_height - 2);
    if (m_hintWidth > m_config.minHintWidth) {
        drawHint(m_offset + m_height - 2);
    }

    m_didScroll = true;
}

void ItemList::scrollDown() {
    if (!m_allowScrolling || m_offset <= 0) {
        return;
    }
    m_offset -= 1;
    ansi.move(0, m_height - 1);
    ansi.moveDownOrScroll();
    if (m_cursor - m_offset >= m_height - 1) {
        m_cursor -= 1;
        drawName(m_offset + m_height - 2);
        if (m_hintWidth > m_config.minHintWidth) {
            drawHint(m_offset + m_height - 2);
        }
    }

    drawName(m_offset);
    if (m_hintWidth > m_config.minHintWidth) {
        drawHint(m_offset);
    }

    m_didScroll = true;
}

void ItemList::moveCursorUp() {
    Item *item = m_itemCache->get(m_cursor + 1);
    if (item == nullptr || item->heuristic == BAD_HEURISTIC) {
        return;
    }

    m_cursor += 1;
    if (m_cursor - m_offset >= m_nVisibleItems) {
        if (!m_allowScrolling) {
            m_cursor -= 1;
            return;
        }
        m_offset += 1;
        ansi.moveHome();
        ansi.moveUpOrScroll();
        m_didScroll = true;
    }
    drawName(m_cursor - 1);
    drawName(m_cursor);
    if (m_hintWidth > m_config.minHintWidth) {
        drawHint(m_cursor - 1);
        drawHint(m_cursor);
    }
}

void ItemList::moveCursorDown() {
    if (m_cursor <= 0) {
        return;
    }
    m_cursor -= 1;
    if (m_cursor - m_offset < 0) {
        if (!m_allowScrolling) {
            m_cursor += 1;
            return;
        }
        m_offset -= 1;

        ansi.move(0, m_height - 1);
        ansi.moveDownOrScroll();

        m_didScroll = true;
    }
    drawName(m_cursor + 1);
    drawName(m_cursor);
    if (m_hintWidth > m_config.minHintWidth) {
        drawHint(m_cursor + 1);
        drawHint(m_cursor);
    }
}

void ItemList::calcVisibleItems() {
    m_nVisibleItems = m_itemCache->size() > m_height - 1
        ? m_height - 1
        : m_itemCache->size();
    for (int i = 0; i < m_nVisibleItems; i++) {
        if (m_itemCache->get(m_offset + i)->heuristic == BAD_HEURISTIC) {
            m_nVisibleItems = i;
            break;
        }
    }
}

void ItemList::resize(int w, int h) {
    bool firstResize = m_width == 0;

    m_width = w;
    m_height = h;

    if (h > m_itemCache->getReserve() * 2) {
        m_itemCache->setReserve(h * 2);
    }

    int selectorWidth = std::max(m_config.selector.size(),
            m_config.activeSelector.size());

    if (m_config.showHints) {
        m_hintWidth = (m_width - ((m_width / 5) * 3) - selectorWidth
            - m_config.minHintSpacing);
        if (m_hintWidth >= m_config.minHintWidth) {
            if (m_hintWidth >= m_config.maxHintWidth) {
                m_hintWidth = m_config.maxHintWidth;
            }
            m_itemWidth = m_width - m_hintWidth - selectorWidth
                - m_config.minHintSpacing;
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
        calcVisibleItems();
        if (m_height - 1 + m_offset > m_itemCache->size()) {
            m_offset = m_itemCache->size() - (m_height - 1);
            if (m_offset < 0) {
                m_offset = 0;
            }
        }
        else if (m_cursor - m_offset >= m_nVisibleItems) {
            m_offset = m_cursor - m_nVisibleItems + 1;
        }

        drawItems();
    }
}

bool ItemList::setSelected(int y) {
    if (m_height - y > m_nVisibleItems) {
        return false;
    }
    int oldCursor = m_cursor;
    m_cursor = m_offset + (m_height - 1 - y);
    drawName(oldCursor);
    drawName(m_cursor);
    if (m_hintWidth > m_config.minHintWidth) {
        drawHint(oldCursor);
        drawHint(m_cursor);
    }
    return true;
}

Item* ItemList::get(int y) const {
    return m_itemCache->get(m_offset + (m_height - 1 - y));
}

bool ItemList::didScroll() {
    bool tmp = m_didScroll;
    m_didScroll = false;
    return tmp;
}

void ItemList::allowScrolling(bool value) {
    m_allowScrolling = value;
}

void ItemList::refresh(bool resetCursor) {
    if (resetCursor) {
        m_cursor = 0;
    }
    m_offset = 0;
    std::vector<int> itemIds(m_nVisibleItems);
    for (int i = 0; i < m_nVisibleItems; i++) {
        itemIds[i] = m_itemCache->get(m_offset + i)->index;
    }

    m_itemCache->refresh();
    calcVisibleItems();
    bool visibleItemsChanged = m_nVisibleItems != itemIds.size();

    if (!visibleItemsChanged) {
        for (int i = 0; i < m_nVisibleItems; i++) {
            Item *item = m_itemCache->get(i + m_offset);
            if (itemIds[i] != item->index) {
                visibleItemsChanged = true;
                break;
            }
        }
    }

    m_logger.log("visibleItemsChanged=%d", visibleItemsChanged);

    if (visibleItemsChanged) {
        calcVisibleItems();
        drawItems();
    }
}

Item* ItemList::getSelected() const {
    if (m_nVisibleItems) {
        return m_itemCache->get(m_cursor);
    }
    return nullptr;
}
