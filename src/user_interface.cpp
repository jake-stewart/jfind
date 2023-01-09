#include "../include/user_interface.hpp"
#include "../include/ansi_wrapper.hpp"

#include <string>

UserInterface::UserInterface() {
    m_outputFile = stdout;
    m_offset = 0;
    m_cursor = 0;
    m_active = true;
    m_selected = false;
}

void UserInterface::setOutputFile(FILE *file) {
    m_outputFile = file;
    m_editor.setOutputFile(file);
}

void UserInterface::setItemCache(ItemCache itemCache) {
    m_itemCache = itemCache;
}

void UserInterface::drawName(int i) {
    std::string name = std::string(m_itemCache.get(i)->text);

    if (name.size() > m_itemWidth) {
        name = name.substr(0, m_itemWidth - 1) + "…";
    }

    move(0, m_height - i - 2 + m_offset);
    if (i == m_cursor) {
        m_styleManager.set(m_config->activeRowStyle);
        clearTilEOL();
        if (m_config->activeSelector.size()) {
            m_styleManager.set(m_config->activeSelectorStyle);
            fprintf(m_outputFile, "%s", m_config->activeSelector.c_str());
        }
        m_styleManager.set(m_config->activeItemStyle);
    }
    else {
        m_styleManager.set(m_config->rowStyle);
        clearTilEOL();
        if (m_config->selector.size()) {
            m_styleManager.set(m_config->selectorStyle);
            fprintf(m_outputFile, "%s", m_config->selector.c_str());
        }
        m_styleManager.set(m_config->itemStyle);
    }

    fprintf(m_outputFile, "%s", name.c_str());
    // fprintf(m_outputFile, "%s %d", name.c_str(), items->get(i).heuristic);
}

void UserInterface::drawHint(int i) {
    char *text = m_itemCache.get(i)->text;
    std::string hint = std::string(text + strlen(text) + 1);

    m_styleManager.set(i == m_cursor ? m_config->activeHintStyle
            : m_config->hintStyle);

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
        move(m_width - hint.size() + idx - 1, m_height - i - 2 + m_offset);
        fprintf(m_outputFile, "…");
        fprintf(m_outputFile, "%s", str + idx);
    }
    else {
        move(m_width - hint.size(), m_height - i - 2 + m_offset);
        fprintf(m_outputFile, "%s", hint.data());
    }
}

void UserInterface::drawItems() {
    if (m_width <= 2 || m_height <= 1) {
        return;
    }

    warmCache();

    for (int i = m_offset; i < m_offset + m_nVisibleItems; i++) {
        drawName(i);
    }

    if (m_hintWidth >= m_config->minHintWidth) {
        for (int i = m_offset; i < m_nVisibleItems + m_offset; i++) {
            drawHint(i);
        }
    }
    if (m_height - m_nVisibleItems - 2 >= 0) {
        move(m_width - 1, m_height - m_nVisibleItems - 2);
        m_styleManager.set(m_config->backgroundStyle);
        clearTilSOF();
    }
}

void UserInterface::drawPrompt() {
    if (!m_width || !m_height) {
        return;
    }
    move(0, m_height - 1);
    m_styleManager.set(m_config->searchRowStyle);
    clearTilEOL();
    m_styleManager.set(m_config->searchPromptStyle);
    fprintf(m_outputFile, "%s", m_config->prompt.c_str());
    move(m_config->prompt.size() + m_config->promptGap, m_height - 1);
    m_styleManager.set(m_config->searchStyle);
}

void UserInterface::drawQuery() {
    if (!m_width || !m_height) {
        return;
    }
    move(m_config->prompt.size() + m_config->promptGap, m_height - 1);
    m_styleManager.set(m_config->searchRowStyle);
    clearTilEOL();
    m_styleManager.set(m_config->searchStyle);
    m_editor.print();
}

void UserInterface::focusEditor() {
    move(m_config->prompt.size() + m_config->promptGap
            + m_editor.getCursorCol(), m_height - 1);
}

void UserInterface::calcVisibleItems() {
    m_nVisibleItems = m_itemCache.size() > m_height - 1
        ? m_height - 1
        : m_itemCache.size();
    for (int i = 0; i < m_nVisibleItems; i++) {
        if (m_itemCache.get(m_offset + i)->heuristic == INT_MAX) {
            m_nVisibleItems = i;
            break;
        }
    }
}

void UserInterface::onResize(int newWidth, int newHeight) {
    bool firstResize = m_width == 0;

    m_width = newWidth;
    m_height = newHeight;

    if (newHeight < m_itemCache.getReserve() * 2) {
        m_itemCache.setReserve(newHeight * 2);
    }

    m_editor.setWidth(m_width - 1);

    int selectorWidth = std::max(m_config->selector.size(),
            m_config->activeSelector.size());

    if (m_config->showHints) {
        m_hintWidth = (m_width - ((m_width / 5) * 3) - selectorWidth - m_config->minHintSpacing);
        if (m_hintWidth >= m_config->minHintWidth) {
            if (m_hintWidth >= m_config->maxHintWidth) {
                m_hintWidth = m_config->maxHintWidth;
            }
            m_itemWidth = m_width - m_hintWidth - selectorWidth - m_config->minHintSpacing;
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

    if (firstResize) {
        return;
    }

    calcVisibleItems();

    if (m_height - 1 + m_offset > m_itemCache.size()) {
        m_offset = m_itemCache.size() - (m_height - 1);
        if (m_offset < 0) {
            m_offset = 0;
        }
    }
    else if (m_cursor - m_offset >= m_nVisibleItems) {
        m_offset = m_cursor - m_nVisibleItems + 1;
    }

    drawItems();
    drawPrompt();
    drawQuery();
    focusEditor();
}

void UserInterface::moveCursorDown() {
    if (m_cursor <= 0) {
        return;
    }
    m_cursor -= 1;
    if (m_cursor - m_offset < 0) {
        m_offset -= 1;

        move(0, m_height - 1);
        moveDownOrScroll();

        drawPrompt();
        drawQuery();
    }
    drawName(m_cursor + 1);
    drawName(m_cursor);
    if (m_hintWidth > m_config->minHintWidth) {
        drawHint(m_cursor + 1);
        drawHint(m_cursor);
    }
}

void UserInterface::warmCache() {
    m_itemCache.get(m_offset);
    m_itemCache.get(m_offset + m_nVisibleItems);
}

void UserInterface::moveCursorUp() {
    Item *item = m_itemCache.get(m_cursor + 1);
    if (item == nullptr || item->heuristic == INT_MAX) {
        return;
    }

    m_cursor += 1;
    if (m_cursor - m_offset >= m_nVisibleItems) {
        m_offset += 1;
        warmCache();
        moveHome();
        moveUpOrScroll();
        drawPrompt();
        drawQuery();
    }
    drawName(m_cursor - 1);
    drawName(m_cursor);
    if (m_hintWidth > m_config->minHintWidth) {
        drawHint(m_cursor - 1);
        drawHint(m_cursor);
    }
}

void UserInterface::scrollUp() {
    Item *item = m_itemCache.get(m_offset + m_height - 1);
    if (item == nullptr || item->heuristic == INT_MAX) {
        return;
    }

    m_offset += 1;
    warmCache();
    moveHome();
    moveUpOrScroll();
    if (m_cursor - m_offset < 0) {
        m_cursor += 1;
        drawName(m_offset);
        if (m_hintWidth > m_config->minHintWidth) {
            drawHint(m_offset);
        }
    }

    drawName(m_offset + m_height - 2);
    if (m_hintWidth > m_config->minHintWidth) {
        drawHint(m_offset + m_height - 2);
    }

    drawPrompt();
    drawQuery();
}

void UserInterface::scrollDown() {
    if (m_offset <= 0) {
        return;
    }
    m_offset -= 1;
    warmCache();
    move(0, m_height - 1);
    moveDownOrScroll();
    if (m_cursor - m_offset >= m_height - 1) {
        m_cursor -= 1;
        drawName(m_offset + m_height - 2);
        if (m_hintWidth > m_config->minHintWidth) {
            drawHint(m_offset + m_height - 2);
        }
    }

    drawName(m_offset);
    if (m_hintWidth > m_config->minHintWidth) {
        drawHint(m_offset);
    }

    drawPrompt();
    drawQuery();
}

void UserInterface::handleClick(int x, int y) {
    bool canDoubleClick = true;
    int newCursor = m_offset + (m_height - 1 - y);
    if (newCursor < 0) {
        return;
    }
    Item *clicked = m_itemCache.get(newCursor);
    if (clicked == nullptr || clicked->heuristic == INT_MAX) {
        return;
    }

    if (m_cursor == newCursor) {
        chrono::milliseconds delta = chrono::duration_cast<chrono
            ::milliseconds>(chrono::system_clock::now() - m_lastClickTime);
        if (canDoubleClick && delta.count() < 250) {
            m_active = false;
            m_selected = true;
        }
    }
    else {
        int oldCursor = m_cursor;
        m_cursor = newCursor;
        drawName(oldCursor);
        drawName(m_cursor);
        if (m_hintWidth > m_config->minHintWidth) {
            drawHint(oldCursor);
            drawHint(m_cursor);
        }
    }
    m_lastClickTime = chrono::system_clock::now();
}

void UserInterface::handleMouse(MouseEvent event) {
    switch (event.button) {
        case MB_SCROLL_UP:
            scrollUp();
            break;
        case MB_SCROLL_DOWN:
            scrollDown();
            break;
        case MB_LEFT:
            if (event.pressed && !event.dragged) {
                if (event.y == m_height) {
                    m_editor.handleClick(event.x - m_config->prompt.size()
                            - m_config->promptGap);
                }
                else {
                    handleClick(event.x, event.y);
                }
            }
            break;
        default:
            break;
    }
}

void UserInterface::handleInput(Key key) {
    switch (key) {
        case K_ESCAPE:
        case K_CTRL_C:
            m_active = false;
            break;

        case 32 ... 126:
            m_editor.input(key);
            break;

        case K_CTRL_A:
            m_editor.input(m_editor.getText());
            break;

        case K_CTRL_H:
        case K_BACKSPACE:
            m_editor.backspace();
            break;

        case K_DELETE:
            m_editor.del();
            break;

        case K_CTRL_U:
            m_editor.clear();
            break;

        case K_UP:
        case K_CTRL_K:
            moveCursorUp();
            break;

        case K_DOWN:
        case K_CTRL_J:
            moveCursorDown();
            break;

        case K_LEFT:
            m_editor.moveCursorLeft();
            break;

        case K_RIGHT:
            m_editor.moveCursorRight();
            break;

        case K_ENTER:
            m_selected = true;
            m_active = false;
            break;

        case K_UTF8:
            m_editor.input(m_reader.getWideChar());
            break;

        case K_MOUSE:
            handleMouse(m_reader.getMouseEvent());
            break;

        default:
            break;
    }
}

void UserInterface::redraw() {
    std::vector<int> itemIds;
    for (int i = 0; i < m_nVisibleItems; i++) {
        Item *item = m_itemCache.get(m_offset + i);
        if (item == nullptr || item->heuristic == INT_MAX) {
            break;
        }
        itemIds.push_back(item->index);
    }

    m_itemCache.refresh();

    calcVisibleItems();
    bool needsRedraw = false;

    if (m_nVisibleItems != itemIds.size()) {
        needsRedraw = true;
    }
    else for (int i = 0; i < m_nVisibleItems; i++) {
        Item *item = m_itemCache.get(i + m_offset);
        if (item == nullptr || itemIds[i] != item->index) {
            needsRedraw = true;
            break;
        }
    }

    if (needsRedraw) {
        m_offset = 0;
        m_cursor = 0;
        drawItems();
        focusEditor();
    }
}

bool UserInterface::isActive() {
    return m_active;
}

Item* UserInterface::getSelected() {
    if (m_selected && m_nVisibleItems) {
        return m_itemCache.get(m_cursor);
    }
    return nullptr;
}

Utf8LineEditor* UserInterface::getEditor() {
    return &m_editor;
}

InputReader* UserInterface::getInputReader() {
    return &m_reader;
}

StyleManager* UserInterface::getStyleManager() {
    return &m_styleManager;
}

void UserInterface::setConfig(Config *config) {
    m_config = config;
}
