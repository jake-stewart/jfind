#ifndef ITEM_PREVIEW_HPP
#define ITEM_PREVIEW_HPP

#include <vector>
#include "ansi_wrapper.hpp"
#include "config.hpp"
#include "item_cache.hpp"
#include "sliding_cache.hpp"
#include "style_manager.hpp"
#include "process.hpp"
#include "buffered_reader.hpp"
#include "item_preview_content.hpp"

class ItemPreview
{
    int m_x;
    int m_y;
    int m_width = 0;
    int m_height = 0;
    int m_offset = 0;
    ItemPreviewContent m_content;
    bool m_optimizeAnsi = false;
    void printLine(int idx);
    void redraw();

public:
    void canOptimizeAnsi(bool value);
    void refresh(ItemPreviewContent content);
    void resize(int x, int y, int w, int h);
    bool scrollUp(int lines);
    bool scrollDown(int lines);
};

#endif
