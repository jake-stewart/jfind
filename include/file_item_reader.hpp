#ifndef FILE_ITEM_READER_HPP
#define FILE_ITEM_READER_HPP

#include <vector>
#include <cstdio>
#include <mutex>
#include <condition_variable>
#include <thread>
#include "item.hpp"
#include "logger.hpp"
#include "event_dispatch.hpp"
#include "double_buffer.hpp"
#include "item_reader.hpp"
#include "interval_thread.hpp"

class FileItemReader : public EventListener {
public:
    FileItemReader(FILE *file);
    bool read();
    void onStart() override;
    void onLoop() override;
    void onEvent(std::shared_ptr<Event> event) override;
    void preOnEvent(EventType eventType) override;
    void dispatchItems();

private:
    void endInterval();

    bool m_itemsRead = true;

    ItemReader m_itemReader;
    IntervalThread m_interval;

    EventDispatch& m_dispatch = EventDispatch::instance();
    Logger m_logger = Logger("FileItemReader");

    DoubleBuffer<std::vector<Item>> m_items;

    bool readFirstBatch();
    bool readWithHints();
    bool readWithoutHints();
};

#endif
