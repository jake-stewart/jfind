#ifndef ITEM_GENERATOR_HPP
#define ITEM_GENERATOR_HPP

#include "logger.hpp"
#include "event_dispatch.hpp"
#include "item_reader.hpp"
#include "interval_thread.hpp"
#include <mutex>

class ItemGenerator : public EventListener {
public:
    ItemGenerator(std::string command);
    void onStart() override;
    void onLoop() override;
    void onEvent(std::shared_ptr<Event> event) override;

    int copyItems(Item *buffer, int idx, int n);
    int size();

private:
    std::mutex m_mut;
    std::string m_query = "";

    IntervalThread m_interval;

    EventDispatch &m_dispatch = EventDispatch::instance();
    Logger m_logger = Logger("ItemGenerator");
    int m_pipefd[2];
    pid_t m_child_pid = -1;
    std::vector<Item> m_items;

    FILE *m_file;
    ItemReader m_itemReader;

    bool m_queryChanged = false;

    bool m_processActive = false;
    bool m_intervalPassed = false;
    bool m_intervalActive = false;

    void dispatchItems();
    void readFirstBatch();

    bool readItem();
    void startChildProcess();
    void endChildProcess();
};

#endif
