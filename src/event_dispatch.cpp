#include "../include/event_dispatch.hpp"

void EventDispatch::subscribe(EventListener *listener, EventType type) {
    m_listeners[type].push_back(listener);
}

void EventDispatch::dispatch(std::shared_ptr<Event> event) {
    for (EventListener *listener : m_listeners[event->getType()]) {
        listener->addEvent(event);
    }
}

EventDispatch &EventDispatch::instance() {
    static EventDispatch singleton;
    return singleton;
}
