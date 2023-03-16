#ifndef ITEM_READER_HPP
#define ITEM_READER_HPP

#include <vector>
#include <stdio.h>
#include <mutex>
#include <condition_variable>
#include "item.hpp"
#include "logger.hpp"
#include "event_dispatch.hpp"
#include "double_buffer.hpp"

class ItemReader : public EventListener {
public:
    ItemReader(FILE *file);
    void setReadHints(bool readHints);
    bool read();
    void onStart();
    void onLoop();
    void onEvent(std::shared_ptr<Event> event);
    void dispatchItems();

private:
    void endInterval();

    bool m_itemsRead = true;
    bool m_intervalPassed = false;
    bool m_intervalActive = false;

    std::thread *m_intervalThread;

    int m_itemId;
    bool m_readHints;
    FILE *m_file;

    EventDispatch& m_dispatch = EventDispatch::instance();
    Logger m_logger = Logger("ItemReader");

    std::mutex m_intervalMut;
    std::condition_variable m_intervalCv;

    DoubleBuffer<std::vector<Item>> m_itemsBuf;

    void intervalThread();

    void readFirstBatch();
    bool readWithHints();
    bool readWithoutHints();
};

#endif
