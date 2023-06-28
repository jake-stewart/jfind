#include "../include/user_interface.hpp"
#include "../include/history_manager.hpp"
#include <chrono>
#include <cstring>
#include <string>

using namespace std::chrono_literals;
using std::chrono::milliseconds;

static void layoutWindows(Pane parent, Placement placement, float p, Pane *first, Pane *second) {
    int totalSize = placement == Placement::Top || placement == Placement::Bottom
        ? parent.h
        : parent.w;
    int offset = placement == Placement::Top || placement == Placement::Bottom
        ? parent.y
        : parent.x;

    int aSize = totalSize * p;
    int bSize = totalSize - aSize;
    int aPos = offset;
    int bPos = offset + aSize;

    if (placement == Placement::Top || placement == Placement::Left) {
        std::swap(aSize, bSize);
        std::swap(aPos, bPos);
    }
    if (placement == Placement::Top || placement == Placement::Bottom) {
        *first = {parent.x, aPos, parent.w, aSize};
        *second = {parent.x, bPos, parent.w, bSize};
    }
    else {
        *first = {aPos, parent.y, aSize, parent.h};
        *second = {bPos, parent.y, bSize, parent.h};
    }
}

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
            // drawQuery();
        }
        return;
    }

    m_spinner.setSpinning(true);
    StyleManager::instance().set(m_config.searchPromptStyle);
    m_spinner.update();
}

void UserInterface::onResize(int w, int h) {
    m_pane.x = 0;
    m_pane.y = 0;
    m_pane.w = w;
    m_pane.h = h;

    bool optimizeAnsi = false;

    if (m_config.externalBorder) {
        border(m_pane);
    }

    if (Config::instance().preview.size()) {
        layoutWindows(
            m_pane, m_config.previewPlacement, 0.5f, &m_itemListPane,
            &m_previewPane
        );
    }
    else {
        m_itemListPane = m_pane;
    }

    m_editorPane.x = m_itemListPane.x;
    m_editorPane.y = m_itemListPane.y + m_itemListPane.h - 1;
    m_editorPane.w = m_itemListPane.w;
    m_editorPane.h = 1;

    if (!m_config.itemsBorder && !m_config.previewBorder || m_config.itemsBorder && (m_config.queryWindow && !m_config.queryBorder)) {
        if (Config::instance().preview.size()) {
            switch (m_config.previewPlacement) {
                case Placement::Top:
                    m_previewPane.h -= 1;
                    horizontalSeperator(
                        m_previewPane.x, m_previewPane.y + m_previewPane.h,
                        m_previewPane.w, m_config.externalBorder,
                        m_config.externalBorder
                    );
                    break;
                case Placement::Bottom:
                    horizontalSeperator(
                        m_previewPane.x, m_previewPane.y, m_previewPane.w,
                        m_config.externalBorder, m_config.externalBorder
                    );
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
    if (m_config.previewBorder && m_config.preview.size()) {
        border(m_previewPane);
    }

    if (m_config.queryWindow) {
        if (m_config.queryBorder) {
            m_itemListPane.h -= 3;
            m_editorPane.h += 2;
            m_editorPane.y -= 2;
            border(m_editorPane);
            if (m_config.itemsBorder) {
                border(m_itemListPane);
            }
        }
        else {
            m_itemListPane.h -= 2;
            if (m_config.itemsBorder) {
                border(m_itemListPane);
            }
            bool connectLeft = m_config.externalBorder ||
                (!m_config.previewBorder &&
                 m_config.previewPlacement == Placement::Left);
            bool connectRight = m_config.externalBorder ||
                (!m_config.previewBorder &&
                 m_config.previewPlacement == Placement::Right);

            if (m_config.preview.size() && m_config.previewBorder && !m_config.itemsBorder) {
                connectRight = false;
                connectLeft = false;
            }

            horizontalSeperator(
                m_editorPane.x, m_editorPane.y - 1, m_editorPane.w, connectLeft,
                connectRight
            );
        }
    }
    else {
        if (m_config.queryBorder) {
            if (m_config.itemsBorder) {
                border(m_itemListPane);
                m_editorPane.y -= 1;
                m_editorPane.x += 1;
                m_editorPane.w -= 2;
            }
            m_itemListPane.h -= 3;
            m_editorPane.h += 2;
            m_editorPane.y -= 2;
            border(m_editorPane);
        }
        else {
            if (m_config.itemsBorder) {
                border(m_itemListPane);
                m_editorPane.y -= 1;
                m_editorPane.x += 1;
                m_editorPane.w -= 2;
            }
            m_itemListPane.h -= 1;
        }
    }

    m_itemPreview->resize(
        m_previewPane.x, m_previewPane.y, m_previewPane.w, m_previewPane.h
    );
    m_itemPreview->canOptimizeAnsi(optimizeAnsi);


    m_itemList->resize(
        m_itemListPane.x, m_itemListPane.y, m_itemListPane.w,
        m_itemListPane.h
    );
    m_itemList->canOptimizeAnsi(optimizeAnsi);


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
                m_itemList->scrollUp();
            }
            if (m_previewPane.pointContained(event.x, event.y)) {
                m_itemPreview->scrollUp();
            }
            break;
        case MB_SCROLL_DOWN:
            if (m_itemListPane.pointContained(event.x, event.y)) {
                m_itemList->scrollDown();
            }
            if (m_previewPane.pointContained(event.x, event.y)) {
                m_itemPreview->scrollDown();
            }
            break;
        case MB_LEFT:
            if (event.pressed && !event.dragged) {
                if (event.y == m_pane.h - 1) {
                    int offset = m_config.prompt.size() + m_config.promptGap;
                    if (event.x >= offset) {
                        m_editor->handleClick(event.x - offset);
                    }
                }
                else {
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
            m_itemList->moveCursorUp();
            break;

        case K_DOWN:
        case K_CTRL_J:
            m_itemList->moveCursorDown();
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
            m_itemPreview->redraw();
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

        if (ms < 10ms && !m_firstUpdate) {
            LOG("sleeping for %lldms", ms.count());
            awaitEvent(10ms - ms);
            return;
        }

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

    milliseconds remaining = m_spinner.frameTimeRemaining();
    if (remaining == 0ms) {
        updateSpinner();
    }
    m_editor->focus();
    fflush(stderr);

    if (!m_spinner.isSpinning()) {
        awaitEvent();
    }
    else if (remaining > 0ms) {
        awaitEvent(remaining);
    }
}

void UserInterface::setThreadsafeReading(bool value) {
    m_threadsafeReading = value;
}
