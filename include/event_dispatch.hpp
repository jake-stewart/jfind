#ifndef EVENT_DISPATCH_HPP
#define EVENT_DISPATCH_HPP

#include "event.hpp"
#include "event_listener.hpp"
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <vector>

class EventDispatch
{
public:
    void dispatch(std::shared_ptr<Event> event);
    void subscribe(EventListener *listener, EventType type);
    static EventDispatch &instance();

private:
    EventDispatch(){};
    std::map<EventType, std::vector<EventListener *>> m_listeners;
};

#endif
