#ifndef INPUT_HANDLER
#define INPUT_HANDLER

#include <string>

class InputHandler
{
private:
    std::string m_query;
    int  m_cursor_pos   = 0;
    int  m_offset       = 0;
    int  m_offset_range = 0;
    bool m_active       = true;
    bool m_selected     = false;


public:
    bool        isActive();
    bool        isSelected();
    void        moveCursor(int delta);
    int         getCursorPos();
    int         getOffset();
    void        setQuery(std::string query);
    std::string getQuery();
    void        handle();
    void        setOffsetRange(int offset_range);
    void        scrollOffset(int delta);
};

#endif
