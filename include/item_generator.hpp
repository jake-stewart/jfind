#ifndef ITEM_GENERATOR_HPP
#define ITEM_GENERATOR_HPP

#include "logger.hpp"
#include "event_dispatch.hpp"
#include <mutex>
#include <thread>

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
    std::thread *m_intervalThread;
    std::mutex m_intervalMut;
    std::condition_variable m_intervalCv;
    std::string m_query = "";

    EventDispatch &m_dispatch = EventDispatch::instance();
    Logger m_logger = Logger("ItemGenerator");
    int m_pipefd[2];
    FILE *m_file = nullptr;
    pid_t m_child_pid = -1;
    int m_itemId = 0;
    std::vector<Item> m_items;

    bool m_queryChanged = false;

    bool m_processActive = false;
    bool m_intervalPassed = false;
    bool m_intervalActive = false;

    void dispatchItems();
    void readFirstBatch();

    void intervalThread();
    void endInterval();
    void startInterval();
    bool readItem();
    void startChildProcess();
    void endChildProcess();
};

#endif
