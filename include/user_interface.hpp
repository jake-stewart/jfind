#ifndef USER_INTERFACE_HPP
#define USER_INTERFACE_HPP

#include "item_cache.hpp"
#include "style_manager.hpp"
#include "config.hpp"
#include "utf8_line_editor.hpp"
#include "input_reader.hpp"
#include "ansi_wrapper.hpp"
#include "sliding_cache.hpp"
#include "event_dispatch.hpp"
#include <chrono>

namespace chrono = std::chrono;

class UserInterface : public EventListener {
public:
    UserInterface(StyleManager *styleManager);
    void drawPrompt();
    void drawQuery();
    void onResize(int w, int h);
    void handleInput(KeyEvent event);
    void redraw();
    void setOutputFile(FILE *file);
    void setItemCache(ItemCache cache);
    void focusEditor();
    void updateSpinner();
    Item* getSelected();
    Utf8LineEditor* getEditor();
    void onEvent(std::shared_ptr<Event> event);
    void onStart();
    void onLoop();

private:
    void quit(bool withSelected);
    void drawName(int i);
    void drawHint(int i);
    void drawItems();
    void drawSpinner();
    void calcVisibleItems();
    void moveCursorDown();
    void moveCursorUp();
    void scrollUp();
    void scrollDown();
    void handleClick(int x, int y);
    void handleMouse(MouseEvent event);
    void warmCache();

    EventDispatch& m_dispatch = EventDispatch::instance();
    Logger& m_logger = Logger::instance();

    std::vector<KeyEvent> m_inputQueue;

    bool m_requiresRedraw;

    bool m_isSorting;
    bool m_isReading;

    int m_spinnerFrame;
    bool m_isSpinning;
    chrono::time_point<chrono::system_clock> m_lastLoopTime;

    FILE *m_outputFile;
    int m_width;
    int m_height;
    int m_offset;
    int m_cursor;
    bool m_selected;
    StyleManager *m_styleManager;
    int m_itemWidth;
    int m_hintWidth;
    int m_nVisibleItems;
    ItemCache m_itemCache;
    Utf8LineEditor m_editor;
    AnsiWrapper &ansi = AnsiWrapper::instance();
    Config& m_config = Config::instance();
    chrono::time_point<chrono::system_clock> m_lastClickTime;
};

#endif
