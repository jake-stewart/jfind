#ifndef EVENT_LISTENER_HPP
#define EVENT_LISTENER_HPP

#include "event.hpp"
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <memory>

namespace chrono = std::chrono;
using chrono::system_clock;

class EventListener {
    bool m_active;
    std::vector<std::shared_ptr<Event>> m_events;
    std::mutex m_mut;

    std::mutex m_cv_mut;
    std::condition_variable m_cv;

    void handleEvents() {
        std::unique_lock lock(m_mut);
        for (std::shared_ptr<Event> event : m_events) {
            onEvent(event);
        }
        m_events.clear();
    }

protected:
    void awaitEvent() {
        std::unique_lock lock(m_cv_mut);
        while (!m_events.size()) {
            m_cv.wait(lock);
        }
    }

    void awaitEvent(std::chrono::milliseconds timeout) {
        chrono::time_point<system_clock> until = system_clock::now() + timeout;
        std::unique_lock lock(m_cv_mut);
        while (system_clock::now() < until && !m_events.size()) {
            m_cv.wait_until(lock, until);
        }
    }

    // preOnEvent is called from a different thread
    // it does not contain event payload and is meant
    // to give the listener a chance to unblock IO for
    // specific events (abort read when quitting)
    virtual void preOnEvent(EventType type) {};

    virtual void onEvent(std::shared_ptr<Event> event) {};
    virtual void onLoop() {};
    virtual void onStart() {};

public:
    void start() {
        m_active = true;

        onStart();
        while (true) {
            handleEvents();
            if (!m_active) {
                break;
            }
            onLoop();
        }
    }

    void end() {
        m_active = false;
    }

    void addEvent(std::shared_ptr<Event> event) {
        std::unique_lock lock(m_mut);
        m_events.push_back(event);
        if (event->getType() == QUIT_EVENT) {
            m_active = false;
        }
        preOnEvent(event->getType());
        std::unique_lock lock2(m_cv_mut);
        m_cv.notify_one();
    }
};

#endif
