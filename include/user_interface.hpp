#ifndef USER_INTERFACE_HPP
#define USER_INTERFACE_HPP

#include "ansi_wrapper.hpp"
#include "config.hpp"
#include "event_dispatch.hpp"
#include "input_reader.hpp"
#include "item_list.hpp"
#include "item_preview.hpp"
#include "spinner.hpp"
#include "style_manager.hpp"
#include "utf8_line_editor.hpp"

struct Pane {
    int x;
    int y;
    int w;
    int h;
    
    bool pointContained(int x, int y) {
        return this->x <= x && x < this->x + w && this->y <= y && y < this->y + h;
    }
};

class UserInterface : public EventListener
{
public:
    UserInterface(ItemList *itemList, Utf8LineEditor *editor);
    void handleInput(KeyEvent *event);
    Item *getSelected() const;
    Key getSelectedKey() const;
    void onEvent(std::shared_ptr<Event> event) override;
    void onLoop() override;
    void onStart() override;
    void setThreadsafeReading(bool value);

private:
    void redraw();
    void updateSpinner();
    void onResize(int w, int h);
    void layoutWindows(float p);
    void drawPrompt();
    void drawQuery();
    bool loadHistory(int direction);
    void handleMouse(MouseEvent event);

    EventDispatch &m_dispatch = EventDispatch::instance();
    AnsiWrapper &ansi = AnsiWrapper::instance();
    const Config &m_config = Config::instance();

    bool m_firstUpdate = true;
    std::chrono::system_clock::time_point m_lastUpdateTime;

    ItemList *m_itemList;
    ItemPreview *m_itemPreview;
    Utf8LineEditor *m_editor;
    Spinner m_spinner;

    bool m_threadsafeReading = true;
    bool m_requestedMoreItems = false;

    std::vector<KeyEvent> m_inputQueue;

    bool m_resetCursor = false;
    bool m_requiresRefresh = false;
    bool m_previewRequiresRefresh = false;
    bool m_isSorting = false;
    bool m_isReading = true;

    Key m_selectedKey = K_NULL;
    bool m_selected = false;

    std::string m_selectedItem = "";

    Pane m_pane;
    Pane m_itemListPane;
    Pane m_previewPane;
    Pane m_editorPane;

    int m_historyIdx = -1;
    std::string m_originalQuery;
};

#endif
