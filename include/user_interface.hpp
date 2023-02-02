#ifndef USER_INTERFACE_HPP
#define USER_INTERFACE_HPP

#include "item_cache.hpp"
#include "style_manager.hpp"
#include "config.hpp"
#include "utf8_line_editor.hpp"
#include "input_reader.hpp"
#include "sliding_cache.hpp"
#include <chrono>

namespace chrono = std::chrono;

class UserInterface {
    public:
        UserInterface();
        void drawPrompt();
        void drawQuery();
        void onResize(int w, int h);
        void handleInput(Key key);
        void redraw();
        bool isActive();
        void setOutputFile(FILE *file);
        void setItemCache(ItemCache cache);
        void focusEditor();
        void updateSpinner(bool isSpinning);
        Item* getSelected();
        Utf8LineEditor* getEditor();
        StyleManager* getStyleManager();
        InputReader* getInputReader();
        void setConfig(Config *config);

    private:
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

        int m_spinnerFrame;
        bool m_isSpinning;

        FILE *m_outputFile;
        int m_width;
        int m_height;
        int m_offset;
        int m_cursor;
        bool m_active;
        bool m_selected;
        StyleManager m_styleManager;
        int m_itemWidth;
        int m_hintWidth;
        int m_nVisibleItems;
        ItemCache m_itemCache;
        InputReader m_reader;
        Utf8LineEditor m_editor;
        Config *m_config;
        chrono::time_point<chrono::system_clock> m_lastClickTime;
};

#endif
