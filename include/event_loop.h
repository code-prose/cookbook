#pragma once

#include "types.h"
#include <queue>
#include <functional>


class EventLoop {
public:
    void AddEvent(Event event);
    void AddHandler(std::function<void(const Event&)> fn);
    void run();

private:
    std::priority_queue<Event, std::vector<Event>, std::greater<Event>> _queue;
    std::function<void(const Event&)> _handler;
};
