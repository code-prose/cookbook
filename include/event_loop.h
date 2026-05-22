#pragma once

#include "types.h"
#include <queue>
#include <functional>


class EventLoop {
public:
    void AddEvent(Event event) { _queue.push(event); };
    void AddHandler(std::function<void(const Event&)> fn) { _handler = fn; };

    void run();

private:
    // might want to write custom ctor to reserver... can get to this during perf
    std::priority_queue<Event, std::vector<Event>, std::greater<Event>> _queue;
    std::function<void(const Event&)> _handler = nullptr;
};
