#include "../include/user_interface.hpp"
#include <chrono>
#include <cstring>
#include <string>

using namespace std::chrono_literals;
using std::chrono::milliseconds;

UserInterface::UserInterface(
    FILE *outputFile, StyleManager *styleManager, ItemList *itemList,
    Utf8LineEditor *editor
)
    : m_spinner(outputFile) {
    m_outputFile = stdout;
    m_selected = false;
    m_isReading = true;
    m_isSorting = false;

    m_outputFile = outputFile;
    m_styleManager = styleManager;
    m_itemList = itemList;
    m_editor = editor;

    m_dispatch.subscribe(this, KEY_EVENT);
    m_dispatch.subscribe(this, RESIZE_EVENT);
    m_dispatch.subscribe(this, ITEMS_SORTED_EVENT);
    m_dispatch.subscribe(this, ALL_ITEMS_READ_EVENT);
}

void UserInterface::onStart() {
    m_logger.log("started");
    m_lastUpdateTime = std::chrono::system_clock::now();
    m_itemList->allowScrolling(m_threadsafeReading);
}

void UserInterface::drawPrompt() {
    ansi.move(0, m_height - 1);
    m_styleManager->set(m_config.searchRowStyle);
    ansi.clearTilEOL();
    m_styleManager->set(m_config.searchPromptStyle);
    fprintf(m_outputFile, "%s", m_config.prompt.c_str());
    ansi.move(m_config.prompt.size() + m_config.promptGap, m_height - 1);
    m_styleManager->set(m_config.searchStyle);
}

void UserInterface::drawQuery() {
    ansi.move(m_config.prompt.size() + m_config.promptGap, m_height - 1);
    m_styleManager->set(m_config.searchRowStyle);
    ansi.clearTilEOL();
    m_styleManager->set(m_config.searchStyle);
    m_editor->print();

    if (m_spinner.isSpinning()) {
        m_styleManager->set(m_config.searchPromptStyle);
        m_spinner.draw();
    }
}

void UserInterface::updateSpinner() {
    bool shouldSpin = m_isReading || m_isSorting;
    if (!shouldSpin) {
        if (m_spinner.isSpinning()) {
            m_spinner.setSpinning(false);
            drawPrompt();
            drawQuery();
        }
        return;
    }

    m_spinner.setSpinning(true);
    m_styleManager->set(m_config.searchPromptStyle);
    m_spinner.update();
}

void UserInterface::focusEditor() {
    ansi.move(
        m_config.prompt.size() + m_config.promptGap + m_editor->getCursorCol(),
        m_height - 1
    );
}

void UserInterface::onResize(int w, int h) {
    m_width = w;
    m_height = h;

    m_itemList->resize(w, h);
    m_editor->setWidth(m_width - 1);
    m_spinner.setPosition(
        m_config.prompt.size() == 1 ? 0 : m_width - 1, m_height - 1
    );

    drawPrompt();
    drawQuery();
    focusEditor();
}

void UserInterface::handleMouse(MouseEvent event) {
    switch (event.button) {
        case MB_SCROLL_UP:
            m_itemList->scrollUp();
            break;
        case MB_SCROLL_DOWN:
            m_itemList->scrollDown();
            break;
        case MB_LEFT:
            if (event.pressed && !event.dragged) {
                if (event.y == m_height) {
                    int offset = m_config.prompt.size() + m_config.promptGap;
                    if (event.x > offset) {
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

void UserInterface::handleInput(KeyEvent event) {
    std::string query = m_editor->getText();

    if (event.getKey() != K_UNKNOWN && event.getKey() != K_NULL) {
        std::vector<int> &keys = Config::instance().additionalKeys;
        for (int key : keys) {
            if (key == event.getKey()) {
                m_selectedKey = event.getKey();
                m_selected = true;
                raise(SIGTERM);
                return;
            }
        }
    }

    switch (event.getKey()) {
        case K_ESCAPE:
        case K_CTRL_C:
            raise(SIGTERM);
            break;

        case 32 ... 126:
            m_editor->input(event.getKey());
            break;

        case K_CTRL_A:
            m_editor->input(m_editor->getText());
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

        case K_LEFT:
            m_editor->moveCursorLeft();
            break;

        case K_RIGHT:
            m_editor->moveCursorRight();
            break;

        case K_ENTER: {
            m_selected = true;
            m_selectedKey = event.getKey();
            raise(SIGTERM);
            break;
        }

        case K_UTF8:
            m_editor->input(event.getWidechar());
            break;

        case K_MOUSE:
            for (MouseEvent mouseEvent : event.getMouseEvents()) {
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

    if (m_itemList->didScroll()) {
        if (!m_requestedMoreItems && m_itemList->getScrollPercentage() > 0.9f) {
            m_requestedMoreItems = true;
            m_dispatch.dispatch(std::make_shared<ItemsRequestEvent>());
        }
        drawPrompt();
        drawQuery();
    }
}

void UserInterface::onEvent(std::shared_ptr<Event> event) {
    m_logger.log("received %s", getEventNames()[event->getType()]);
    switch (event->getType()) {
        case KEY_EVENT: {
            KeyEvent *keyEvent = (KeyEvent *)event.get();
            m_inputQueue.push_back(*keyEvent);
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
    if (m_inputQueue.size()) {
        for (KeyEvent &event : m_inputQueue) {
            handleInput(event);
        }
        m_inputQueue.clear();
        if (m_editor->requiresRedraw()) {
            drawQuery();
        }
    }
    if (m_requiresRefresh) {
        std::chrono::system_clock::time_point
            now = std::chrono::system_clock::now();
        std::chrono::system_clock::time_point::duration duration = now -
            m_lastUpdateTime;
        std::chrono::milliseconds
            ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration
            );

        if (ms < 10ms && !m_firstUpdate) {
            awaitEvent(10ms - ms);
            return;
        }

        m_lastUpdateTime = now;
        m_firstUpdate = false;
        m_logger.log("refreshed after %lldms", ms.count());
        m_itemList->refresh(m_resetCursor);
        m_resetCursor = false;
        m_requiresRefresh = false;
    }

    milliseconds remaining = m_spinner.frameTimeRemaining();
    if (remaining == 0ms) {
        updateSpinner();
    }
    focusEditor();
    fflush(m_outputFile);

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
