#ifndef ITEM_PREVIEW_READER_HPP
#define ITEM_PREVIEW_READER_HPP

#include <vector>
#include <mutex>
#include <regex>
#include <optional>
#include "event_listener.hpp"
#include "event_dispatch.hpp"
#include "process.hpp"
#include "item.hpp"
#include "interval_thread.hpp"
#include "buffered_reader.hpp"
#include "item_preview_content.hpp"
#include "ansi_string_parser.hpp"

class ItemPreviewReader : public EventListener
{
    ItemPreviewContent m_content;
    Process m_process;
    BufferedReader m_reader;
    EventDispatch &m_dispatch = EventDispatch::instance();
    AnsiStringParser m_ansiStringParser;
    std::optional<std::string> m_newItem;
    IntervalThread m_interval;
    std::optional<std::regex> m_lineRegex;
    bool m_firstPreviewRead = false;

    void setItem(Item *item);
    bool readLine();

public:
    void onStart() override;
    void onEvent(std::shared_ptr<Event> event) override;
    void onLoop() override;

    ItemPreviewReader();
};

#endif
