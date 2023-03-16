#ifndef USER_INTERFACE_HPP
#define USER_INTERFACE_HPP

#include "style_manager.hpp"
#include "config.hpp"
#include "utf8_line_editor.hpp"
#include "input_reader.hpp"
#include "ansi_wrapper.hpp"
#include "event_dispatch.hpp"
#include "spinner.hpp"
#include "item_list.hpp"

class UserInterface : public EventListener {
public:
    UserInterface(FILE *outputFile, StyleManager *styleManager,
            ItemList *itemLis, Utf8LineEditor *editor);
    void handleInput(KeyEvent event);
    Item* getSelected();
    void onEvent(std::shared_ptr<Event> event);
    void onLoop();

private:
    void redraw();
    void focusEditor();
    void updateSpinner();
    void onResize(int w, int h);
    void drawPrompt();
    void drawQuery();
    void handleMouse(MouseEvent event);

    EventDispatch& m_dispatch = EventDispatch::instance();
    Logger m_logger = Logger("UserInterface");
    StyleManager *m_styleManager;
    AnsiWrapper& ansi = AnsiWrapper::instance();
    Config& m_config = Config::instance();
    FILE *m_outputFile;

    ItemList *m_itemList;
    Utf8LineEditor *m_editor;
    Spinner m_spinner;

    std::vector<KeyEvent> m_inputQueue;

    bool m_requiresRefresh;
    bool m_isSorting = false;
    bool m_isReading = true;

    bool m_selected = false;

    int m_width = 0;
    int m_height = 0;
};

#endif
