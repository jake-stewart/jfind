#ifndef EVENT_LISTENER_HPP
#define EVENT_LISTENER_HPP

#include "event.hpp"
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <memory>

class EventListener {
    bool m_active = false;
    std::vector<std::shared_ptr<Event>> m_events;
    std::mutex m_mut;

    std::mutex m_cv_mut;
    std::condition_variable m_cv;

    void handleEvents() {
        std::unique_lock lock(m_mut);
        for (std::shared_ptr<Event> event : m_events) {
            onEvent(event);
            if (event->getType() == QUIT_EVENT) {
                m_active = false;
            }
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
        std::chrono::time_point<std::chrono::system_clock> until
            = std::chrono::system_clock::now() + timeout;
        std::unique_lock lock(m_cv_mut);
        while (std::chrono::system_clock::now() < until && !m_events.size()) {
            m_cv.wait_until(lock, until);
        }
    }

    // preOnEvent is called from a different thread
    // it does not contain event payload and is meant
    // to give the listener a chance to unblock IO for
    // specific events (abort read when quitting)
    virtual void preOnEvent(EventType type) {};

    virtual void onEvent(std::shared_ptr<Event> event) {};
    virtual void onLoop() {
        return awaitEvent();
    };
    virtual void onStart() {};

public:
    void start() {
        m_active = true;

        onStart();
        while (m_active) {
            onLoop();
            handleEvents();
        }
    }

    void end() {
        m_active = false;
    }

    void addEvent(std::shared_ptr<Event> event) {
        preOnEvent(event->getType());
        {
            std::unique_lock lock(m_mut);
            m_events.push_back(event);
        }
        {
            std::unique_lock lock(m_cv_mut);
            m_cv.notify_one();
        }
    }
};

#endif
