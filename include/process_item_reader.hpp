#ifndef ITEM_GENERATOR_HPP
#define ITEM_GENERATOR_HPP

#include "double_buffer.hpp"
#include "event_dispatch.hpp"
#include "interval_thread.hpp"
#include "item_reader.hpp"
#include "process.hpp"
#include <mutex>
#include <string>

class ProcessItemReader : public EventListener
{
public:
    ProcessItemReader(std::string command, std::string startQuery);
    void onStart() override;
    void onLoop() override;
    void onEvent(std::shared_ptr<Event> event) override;

    int copyItems(Item *buffer, int idx, int n);
    int size();
    const std::vector<Item> &getItems();

private:
    std::mutex m_mut;
    std::string m_newQuery;
    std::string m_query;

    Process m_process;
    IntervalThread m_interval;

    EventDispatch &m_dispatch = EventDispatch::instance();
    DoubleBuffer<std::vector<Item>> m_items;
    ItemReader m_itemReader;

    std::vector<char *> m_previousReaderBuffers;

    int m_readMax;
    bool m_queryChanged = false;

    bool m_intervalPassed = false;
    bool m_intervalActive = false;

    void dispatchItems();
    void dispatchAllRead(bool value);

    bool readFirstBatch();
    bool readItem();

    void startChildProcess();
};

#endif
