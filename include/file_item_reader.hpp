#ifndef FILE_ITEM_READER_HPP
#define FILE_ITEM_READER_HPP

#include "double_buffer.hpp"
#include "event_dispatch.hpp"
#include "interval_thread.hpp"
#include "item.hpp"
#include "item_reader.hpp"
#include <condition_variable>
#include <cstdio>
#include <mutex>
#include <thread>
#include <vector>

class FileItemReader : public EventListener
{
public:
    FileItemReader(FILE *file);
    bool read();
    void onStart() override;
    void onLoop() override;
    void onEvent(std::shared_ptr<Event> event) override;
    void dispatchItems();

private:
    void endInterval();

    bool m_itemsRead = true;
    FILE *m_file;

    ItemReader m_itemReader;
    IntervalThread m_interval;

    EventDispatch &m_dispatch = EventDispatch::instance();

    DoubleBuffer<std::vector<Item>> m_items;

    bool readFirstBatch();
    bool readWithHints();
    bool readWithoutHints();
};

#endif
