#include "../include/user_interface.hpp"
#include "../include/history_manager.hpp"
#include <chrono>
#include <cstring>
#include <string>

using namespace std::chrono_literals;
using std::chrono::milliseconds;

static void horizontalSeperator(int col, int row, int size) {
    StyleManager::instance().set(Config::instance().borderStyle);
    AnsiWrapper::instance().move(col, row);
    for (int i = 0; i < size; i++) {
        fprintf(
            stderr, "%s",
            Config::instance().borderChars[HORIZONTAL_BORDER].c_str()
        );
    }
}

static void horizontalSeperator(
    int col, int row, int size, bool connectLeft, bool connectRight
) {
    StyleManager::instance().set(Config::instance().borderStyle);
    AnsiWrapper::instance().move(col - connectLeft, row);
    if (connectLeft) {
        fprintf(
            stderr, "%s",
            Config::instance().borderChars[LEFT_JOIN_BORDER].c_str()
        );
    }
    for (int i = 0; i < size; i++) {
        fprintf(
            stderr, "%s",
            Config::instance().borderChars[HORIZONTAL_BORDER].c_str()
        );
    }
    if (connectRight) {
        fprintf(
            stderr, "%s",
            Config::instance().borderChars[RIGHT_JOIN_BORDER].c_str()
        );
    }
}

static void verticalSeperator(int col, int row, int size) {
    StyleManager::instance().set(Config::instance().borderStyle);
    AnsiWrapper::instance().move(col, row);
    for (int i = 0; i < size; i++) {
        fprintf(
            stderr, "%s",
            Config::instance().borderChars[VERTICAL_BORDER].c_str()
        );
        AnsiWrapper::instance().moveLeft();
        AnsiWrapper::instance().moveDown();
    }
}

static void verticalSeperator(int col, int row, int size, bool connectTop, bool connectBottom) {
    StyleManager::instance().set(Config::instance().borderStyle);
    AnsiWrapper::instance().move(col, row - connectTop);
    if (connectTop) {
        fprintf(
            stderr, "%s",
            Config::instance().borderChars[TOP_JOIN_BORDER].c_str()
        );
        AnsiWrapper::instance().moveLeft();
        AnsiWrapper::instance().moveDown();
    }
    for (int i = 0; i < size; i++) {
        fprintf(
            stderr, "%s",
            Config::instance().borderChars[VERTICAL_BORDER].c_str()
        );
        AnsiWrapper::instance().moveLeft();
        AnsiWrapper::instance().moveDown();
    }
    if (connectBottom) {
        fprintf(
            stderr, "%s",
            Config::instance().borderChars[BOTTOM_JOIN_BORDER].c_str()
        );
    }
}

static void border(Pane &pane) {
    pane.x += 1;
    pane.y += 1;
    pane.w -= 2;
    pane.h -= 2;
    StyleManager::instance().set(Config::instance().borderStyle);
    AnsiWrapper::instance().move(pane.x - 1, pane.y - 1);
    fprintf(
        stderr, "%s", Config::instance().borderChars[TOP_LEFT_BORDER].c_str()
    );
    for (int i = 0; i < pane.w; i++) {
        fprintf(
            stderr, "%s",
            Config::instance().borderChars[HORIZONTAL_BORDER].c_str()
        );
    }
    fprintf(
        stderr, "%s", Config::instance().borderChars[TOP_RIGHT_BORDER].c_str()
    );
    for (int i = 0; i < pane.h; i++) {
        AnsiWrapper::instance().move(pane.x - 1, pane.y + i);
        fprintf(
            stderr, "%s",
            Config::instance().borderChars[VERTICAL_BORDER].c_str()
        );
        AnsiWrapper::instance().move(pane.x + pane.w, pane.y + i);
        fprintf(
            stderr, "%s",
            Config::instance().borderChars[VERTICAL_BORDER].c_str()
        );
    }
    AnsiWrapper::instance().move(pane.x - 1, pane.y + pane.h);
    fprintf(
        stderr, "%s", Config::instance().borderChars[BOTTOM_LEFT_BORDER].c_str()
    );
    for (int i = 0; i < pane.w; i++) {
        fprintf(
            stderr, "%s",
            Config::instance().borderChars[HORIZONTAL_BORDER].c_str()
        );
    }
    fprintf(
        stderr, "%s",
        Config::instance().borderChars[BOTTOM_RIGHT_BORDER].c_str()
    );
}

UserInterface::UserInterface(ItemList *itemList, Utf8LineEditor *editor) {
    m_selected = false;
    m_isReading = true;
    m_isSorting = false;

    m_itemList = itemList;
    m_editor = editor;

    m_itemPreview = new ItemPreview();

    m_dispatch.subscribe(this, KEY_EVENT);
    m_dispatch.subscribe(this, RESIZE_EVENT);
    m_dispatch.subscribe(this, ITEMS_SORTED_EVENT);
    m_dispatch.subscribe(this, ALL_ITEMS_READ_EVENT);
    m_dispatch.subscribe(this, PREVIEW_READ_EVENT);
}

void UserInterface::onStart() {
    LOG("started");
    m_lastUpdateTime = std::chrono::system_clock::now();
    m_itemList->allowScrolling(m_threadsafeReading);
    m_itemList->allowWrapping(m_config.wrapItemList && !m_config.command.size());
}

void UserInterface::drawPrompt() {
    ansi.move(m_editorPane.x, m_editorPane.y);
    StyleManager::instance().set(m_config.searchRowStyle);
    StyleManager::instance().set(m_config.searchPromptStyle);
    fprintf(stderr, "%s", m_config.prompt.c_str());
}

void UserInterface::drawQuery() {
    StyleManager::instance().set(m_config.searchStyle);
    m_editor->redraw();
    if (m_spinner.isSpinning()) {
        StyleManager::instance().set(m_config.searchPromptStyle);
        m_spinner.draw();
    }
}

void UserInterface::updateSpinner() {
    bool shouldSpin = m_isReading || m_isSorting;
    if (!shouldSpin) {
        if (m_spinner.isSpinning()) {
            m_spinner.setSpinning(false);
            drawPrompt();
        }
        return;
    }
    m_spinner.setSpinning(true);
    milliseconds remaining = m_spinner.frameTimeRemaining();
    if (remaining <= 0ms) {
        StyleManager::instance().set(m_config.searchPromptStyle);
        m_spinner.update();
    }
}

void UserInterface::layoutWindows(float p, Placement placement, bool vertical) {
    bool backwards = placement == Placement::Top || placement == Placement::Left;

    int totalSize = vertical ? m_pane.h : m_pane.w;
    int offset = vertical ? m_pane.y : m_pane.x;

    int previewSize = totalSize * p;
    int itemListSize = totalSize - previewSize;
    int itemListPos = offset;
    int previewPos = offset + itemListSize;
    if (backwards) {
        previewPos = offset;
        itemListPos = offset + previewSize;
    }
    if (vertical) {
        m_itemListPane = {m_pane.x, itemListPos, m_pane.w, itemListSize};
        m_previewPane = {m_pane.x, previewPos, m_pane.w, previewSize};
    }
    else {
        m_itemListPane = {itemListPos, m_pane.y, itemListSize, m_pane.h};
        m_previewPane = {previewPos, m_pane.y, previewSize, m_pane.h};
    }
}

void UserInterface::onResize(int w, int h) {
    // todo: this function gotta be cleaned up good
    m_pane.x = 0;
    m_pane.y = 0;
    m_pane.w = w;
    m_pane.h = h;

    bool previewBorder;
    bool itemsBorder;
    bool queryBorder;
    bool queryWindow;

    switch (m_config.windowStyle) {
        case WindowStyle::Compact:
            previewBorder = false;
            itemsBorder = false;
            queryBorder = false;
            queryWindow = false;
            break;
        case WindowStyle::Merged:
            previewBorder = false;
            itemsBorder = false;
            queryBorder = false;
            queryWindow = true;
            break;
        case WindowStyle::Windowed:
            previewBorder = true;
            itemsBorder = true;
            queryBorder = true;
            queryWindow = true;
            break;
    }

    int minWidth = 10 + queryBorder + itemsBorder * 2 +
        m_config.externalBorder * 2;

    int minHeight = 5 + queryBorder + itemsBorder * 2 +
        m_config.externalBorder * 2;

    bool small = h < minHeight || w < minWidth;
    float p = small ? 0.0f : Config::instance().percentPreview;

    previewBorder &= !small;
    itemsBorder &= !small;
    queryBorder &= !small;
    queryWindow &= !small;

    Placement placement = m_config.previewPlacement;
    bool vertical = placement == Placement::Top || placement == Placement::Bottom;
    if (!vertical && Config::instance().previewMinWidth > 0
            && Config::instance().previewMinWidth > m_pane.w * p) {
        placement = placement == Placement::Right ? Placement::Bottom : Placement::Top;
        vertical = true;
    }

    if (m_config.preview.size() &&
        (placement == Placement::Left ||
         placement == Placement::Right)) {
        minWidth += 2 + previewBorder;
    }

    if (m_config.preview.size() &&
        (placement == Placement::Top ||
         placement == Placement::Bottom)) {
        minHeight += 2 + previewBorder;
    }

    bool optimizeAnsi = (vertical || !Config::instance().preview.size());

    if (m_config.externalBorder) {
        optimizeAnsi = false;
        border(m_pane);
    }

    if (Config::instance().preview.size()) {
        layoutWindows(p, placement, vertical);
    }
    else {
        m_itemListPane = m_pane;
    }

    m_editorPane.x = m_itemListPane.x;
    if (m_config.queryPlacement == VerticalPlacement::Bottom) {
        m_editorPane.y = m_itemListPane.y + m_itemListPane.h - 1;
    }
    else {
        m_editorPane.y = m_itemListPane.y;
    }
    m_editorPane.w = m_itemListPane.w;
    m_editorPane.h = 1;

    if (!itemsBorder && !previewBorder ||
        itemsBorder &&
            (queryWindow && !queryBorder)) {
        if (Config::instance().preview.size()) {
            switch (placement) {
                case Placement::Top:
                    m_previewPane.h -= 1;
                    if (m_previewPane.h > 0) {
                        horizontalSeperator(
                            m_previewPane.x, m_previewPane.y + m_previewPane.h,
                            m_previewPane.w, m_config.externalBorder,
                            m_config.externalBorder
                        );
                    }
                    break;
                case Placement::Bottom:
                    if (m_previewPane.h > 0) {
                        horizontalSeperator(
                            m_previewPane.x, m_previewPane.y, m_previewPane.w,
                            m_config.externalBorder, m_config.externalBorder
                        );
                    }
                    m_previewPane.h -= 1;
                    m_previewPane.y += 1;
                    break;
                case Placement::Left:
                    m_previewPane.w -= 1;
                    verticalSeperator(
                        m_previewPane.x + m_previewPane.w, m_previewPane.y,
                        m_previewPane.h, m_config.externalBorder,
                        m_config.externalBorder
                    );
                    break;
                case Placement::Right:
                    verticalSeperator(
                        m_previewPane.x, m_previewPane.y, m_previewPane.h,
                        m_config.externalBorder, m_config.externalBorder
                    );
                    m_previewPane.w -= 1;
                    m_previewPane.x += 1;
                    break;
            }
        }
    }
    if (previewBorder && m_config.preview.size()) {
        border(m_previewPane);
    }

    if (queryWindow) {
        if (queryBorder) {
            if (m_config.queryPlacement == VerticalPlacement::Top) {
                m_itemListPane.h -= 3;
                m_itemListPane.y += 3;
                m_editorPane.h += 2;
            }
            else {
                m_itemListPane.h -= 3;
                m_editorPane.h += 2;
                m_editorPane.y -= 2;
            }
            border(m_editorPane);
            if (itemsBorder) {
                border(m_itemListPane);
            }
        }
        else {
            if (m_config.queryPlacement == VerticalPlacement::Top) {
                m_itemListPane.y += 2;
                m_itemListPane.h -= 2;
            }
            else {
                m_itemListPane.h -= 2;
            }
            if (itemsBorder) {
                border(m_itemListPane);
            }
            bool connectLeft = m_config.externalBorder ||
                (!previewBorder && placement == Placement::Left);
            bool connectRight = m_config.externalBorder ||
                (!previewBorder && placement == Placement::Right);

            if (m_config.preview.size() && previewBorder &&
                    !itemsBorder && !vertical) {
                connectRight = false;
                connectLeft = false;
            }

            if (m_config.queryPlacement == VerticalPlacement::Top) {
                horizontalSeperator(
                    m_editorPane.x, m_editorPane.y + 1, m_editorPane.w, connectLeft,
                    connectRight
                );
            }
            else {
                horizontalSeperator(
                    m_editorPane.x, m_editorPane.y - 1, m_editorPane.w, connectLeft,
                    connectRight
                );
            }
        }
    }
    else {
        if (queryBorder) {
            if (itemsBorder) {
                border(m_itemListPane);
                if (m_config.queryPlacement == VerticalPlacement::Top) {
                    m_editorPane.y += 1;
                    m_editorPane.x += 1;
                    m_editorPane.w -= 2;
                }
                else {
                    m_editorPane.y -= 1;
                    m_editorPane.x += 1;
                    m_editorPane.w -= 2;
                }
            }
            if (m_config.queryPlacement == VerticalPlacement::Top) {
                m_itemListPane.h -= 3;
                m_editorPane.h += 2;
                m_itemListPane.y -= 2;
            }
            else {
                m_itemListPane.h -= 3;
                m_editorPane.h += 2;
                m_editorPane.y -= 2;
            }

            border(m_editorPane);
        }
        else {
            if (itemsBorder) {
                border(m_itemListPane);
                if (m_config.queryPlacement == VerticalPlacement::Top) {
                    m_editorPane.y += 1;
                    m_editorPane.x += 1;
                    m_editorPane.w -= 2;
                }
                else {
                    m_editorPane.y -= 1;
                    m_editorPane.x += 1;
                    m_editorPane.w -= 2;
                }
            }
            if (m_config.queryPlacement == VerticalPlacement::Top) {
                m_itemListPane.y += 1;
                m_itemListPane.h -= 1;
            }
            else {
                m_itemListPane.h -= 1;
            }
        }
    }

    LOG("optimizeAnsi=%d", optimizeAnsi);

    m_itemPreview->canOptimizeAnsi(optimizeAnsi && !previewBorder);
    m_itemPreview->resize(
        m_previewPane.x, m_previewPane.y, m_previewPane.w, m_previewPane.h
    );

    m_itemList->canOptimizeAnsi(optimizeAnsi && !itemsBorder);
    m_itemList->resize(
        m_itemListPane.x, m_itemListPane.y, m_itemListPane.w,
        m_itemListPane.h
    );

    int promptSize = m_config.prompt.size() + m_config.promptGap;
    int spinnerReservation = m_config.prompt.size() == 1 ? 0 : 2;
    m_editor->resize(
        m_editorPane.x + promptSize,
        m_editorPane.y,
        m_editorPane.w - promptSize - spinnerReservation
    );

    m_spinner.setPosition(
        m_config.prompt.size() == 1
            ? m_editorPane.x
            : m_editorPane.x + m_editorPane.w - 1,
        m_editorPane.y
    );

    drawPrompt();
    drawQuery();
    m_editor->focus();
}

void UserInterface::handleMouse(MouseEvent event) {
    switch (event.button) {
        case MB_SCROLL_UP:
            if (m_itemListPane.pointContained(event.x, event.y)) {
                if (m_config.queryPlacement == VerticalPlacement::Top) {
                    m_itemList->scrollDown();
                }
                else {
                    m_itemList->scrollUp();
                }
            }
            if (m_previewPane.pointContained(event.x, event.y)) {
                m_itemPreview->scrollUp(1);
            }
            break;
        case MB_SCROLL_DOWN:
            if (m_itemListPane.pointContained(event.x, event.y)) {
                if (m_config.queryPlacement == VerticalPlacement::Top) {
                    m_itemList->scrollUp();
                }
                else {
                    m_itemList->scrollDown();
                }
            }
            if (m_previewPane.pointContained(event.x, event.y)) {
                m_itemPreview->scrollDown(1);
            }
            break;
        case MB_LEFT:
            if (!event.pressed || event.dragged) {
                return;
            }
            if (m_itemListPane.pointContained(event.x, event.y)) {
                if (event.numClicks >= 2 &&
                    m_itemList->get(event.y) == m_itemList->getSelected()) {
                    m_selected = true;
                    m_selectedKey = K_MOUSE;
                    raise(SIGTERM);
                }
                else {
                    m_itemList->setSelected(event.y);
                }
            }
            else if (m_editorPane.pointContained(event.x, event.y)) {
                int offset = m_config.prompt.size() + m_config.promptGap;
                m_editor->handleClick(event.x - m_editorPane.x - offset);
            }
            break;
        default:
            break;
    }
}

bool UserInterface::loadHistory(int direction) {
    if (direction == 1) {
        if (m_historyIdx == -1) {
            m_originalQuery = m_editor->getText();
        }
    }
    else {
        if (m_historyIdx == 0) {
            m_historyIdx--;
            m_editor->clear();
            m_editor->input(m_originalQuery);
            return true;
        }
    }
    std::string *entry;

    if (HistoryManager::instance().getEntry(m_historyIdx + direction, &entry)) {
        m_historyIdx += direction;
        m_editor->clear();
        m_editor->input(*entry);
        return true;
    }
    return false;
}

void UserInterface::handleInput(KeyEvent *event) {
    std::string query = m_editor->getText();

    bool canAccept = m_itemList->getSelected() ||
        Config::instance().acceptNonMatch;

    if (event->getKey() != K_UNKNOWN && event->getKey() != K_NULL) {
        std::vector<int> &keys = Config::instance().additionalKeys;
        for (int key : keys) {
            if (key == event->getKey()) {
                if (!canAccept) {
                    return;
                }
                m_selectedKey = event->getKey();
                m_selected = true;
                raise(SIGTERM);
                return;
            }
        }
    }

    switch (event->getKey()) {
        case K_ESCAPE:
        case K_CTRL_C:
            raise(SIGTERM);
            break;

        case 32 ... 126:
            m_editor->input(event->getKey());
            break;

        case K_CTRL_V:
            m_editor->input(m_editor->getText());
            break;

        case K_CTRL_E:
            m_editor->moveCursorEndOfLine();
            break;

        case K_CTRL_W:
            m_editor->backspaceWord();
            break;

        case K_CTRL_A:
            m_editor->moveCursorStartOfLine();
            break;

        case K_CTRL_H:
        case K_BACKSPACE:
            m_editor->backspace();
            break;

        case K_DELETE:
            m_editor->del();
            break;

        case K_CTRL_U:
            m_editor->clear();
            break;

        case K_UP:
        case K_CTRL_K:
            if (m_config.queryPlacement == VerticalPlacement::Top) {
                m_itemList->moveCursorDown();
            }
            else {
                m_itemList->moveCursorUp();
            }
            break;

        case K_DOWN:
        case K_CTRL_J:
            if (m_config.queryPlacement == VerticalPlacement::Top) {
                m_itemList->moveCursorUp();
            }
            else {
                m_itemList->moveCursorDown();
            }
            break;

        case K_PAGE_UP:
        case K_ALT_B:
        case K_ALT_b:
            if (m_itemPreview) {
                m_itemPreview->scrollUp(m_previewPane.h);
            }
            break;

        case K_PAGE_DOWN:
        case K_ALT_F:
        case K_ALT_f:
            if (m_itemPreview) {
                m_itemPreview->scrollDown(m_previewPane.h);
            }
            break;

        case K_ALT_U:
        case K_ALT_u:
            if (m_itemPreview) {
                m_itemPreview->scrollUp(10);
            }
            break;

        case K_ALT_D:
        case K_ALT_d:
            if (m_itemPreview) {
                m_itemPreview->scrollDown(10);
            }
            break;

        case K_CTRL_P:
            loadHistory(1);
            break;

        case K_CTRL_N:
            loadHistory(-1);
            break;

        case K_LEFT:
        case K_CTRL_B:
            m_editor->moveCursorLeft();
            break;

        case K_RIGHT:
        case K_CTRL_F:
            m_editor->moveCursorRight();
            break;

        case K_ENTER: {
            if (!canAccept) {
                break;
            }
            m_selected = true;
            m_selectedKey = event->getKey();
            raise(SIGTERM);
            break;
        }

        case K_UTF8:
            m_editor->input(event->getWidechar());
            break;

        case K_MOUSE:
            for (MouseEvent mouseEvent : event->getMouseEvents()) {
                handleMouse(mouseEvent);
            }
            break;

        default:
            break;
    }

    if (m_editor->getText() != query) {
        m_isSorting = true;
        m_resetCursor = true;
        m_dispatch.dispatch(
            std::make_shared<QueryChangeEvent>(m_editor->getText())
        );
    }

    if (!m_requestedMoreItems && m_itemList->getScrollPercentage() > 0.9f) {
        m_requestedMoreItems = true;
        m_dispatch.dispatch(std::make_shared<ItemsRequestEvent>());
    }
}

void UserInterface::onEvent(std::shared_ptr<Event> event) {
    LOG("received %s", getEventNames()[event->getType()]);
    switch (event->getType()) {
        case KEY_EVENT: {
            KeyEvent *keyEvent = (KeyEvent *)event.get();
            handleInput(keyEvent);
            break;
        }
        case RESIZE_EVENT: {
            ResizeEvent *resizeEvent = (ResizeEvent *)event.get();
            onResize(resizeEvent->getWidth(), resizeEvent->getHeight());
            break;
        }
        case ITEMS_SORTED_EVENT: {
            ItemsSortedEvent *sortedEvent = (ItemsSortedEvent *)event.get();
            m_requiresRefresh = true;
            m_requestedMoreItems = false;
            m_isSorting = sortedEvent->getQuery() != m_editor->getText();
            break;
        }
        case ALL_ITEMS_READ_EVENT: {
            AllItemsReadEvent *itemsEvent = (AllItemsReadEvent *)event.get();
            bool canScroll = m_threadsafeReading || itemsEvent->getValue();
            m_itemList->allowScrolling(canScroll);
            m_isReading = !itemsEvent->getValue();
            break;
        }
        case PREVIEW_READ_EVENT: {
            PreviewReadEvent *readEvent = (PreviewReadEvent *)event.get();
            m_itemPreview->refresh(readEvent->getContent());
            break;
        }
        default:
            break;
    }
}

Item *UserInterface::getSelected() const {
    if (m_selected) {
        return m_itemList->getSelected();
    }
    return nullptr;
}

Key UserInterface::getSelectedKey() const {
    return m_selectedKey;
}

void UserInterface::onLoop() {
    if (m_editor->requiresRedraw()) {
        drawQuery();
    }

    // for (KeyEvent &keyEvent : m_inputQueue) {
    //     handleInput(&keyEvent);
    // }
    // m_inputQueue.clear();

    if (m_requiresRefresh) {
        std::chrono::system_clock::time_point
            now = std::chrono::system_clock::now();
        std::chrono::system_clock::time_point::duration duration = now -
            m_lastUpdateTime;
        std::chrono::milliseconds
            ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration
            );

        // if (ms < 10ms) {
        //     LOG("sleeping for %lldms", ms.count());
        //     awaitEvent(1ms - ms);
        //     return;
        // }

        m_lastUpdateTime = now;
        m_firstUpdate = false;
        LOG("refreshed after %lldms", ms.count());
        m_itemList->refresh(m_resetCursor);
        m_resetCursor = false;
        m_requiresRefresh = false;
    }

    std::string newSelectedItem = m_itemList->getSelected()
        ? (m_config.selectHint ? m_itemList->getSelected()->text +
                   strlen(m_itemList->getSelected()->text) + 1
                               : m_itemList->getSelected()->text)
        : "";

    if (m_selectedItem != newSelectedItem) {
        LOG("dispatching selected item change");
        m_selectedItem = newSelectedItem;
        Item *selected = m_itemList->getSelected();
        if (selected == nullptr) {
            m_itemPreview->refresh({});
        }
        else {
            m_dispatch.dispatch(
                std::make_shared<SelectedItemChangeEvent>(newSelectedItem)
            );
        }
    }

    updateSpinner();
    m_editor->focus();
    fflush(stderr);

    if (!m_spinner.isSpinning()) {
        return awaitEvent();
    }
    awaitEvent(m_spinner.frameTimeRemaining());
}

void UserInterface::setThreadsafeReading(bool value) {
    m_threadsafeReading = value;
}
