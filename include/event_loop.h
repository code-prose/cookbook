#pragma once

#include "types.h"
#include <queue>
#include <functional>
#include <utility>


class EventLoop {
public:
    void AddEvent(const Event& event) { _queue.push(event); };
    void AddHandler(std::function<void(const Event&)> fn) { _handler = std::move(fn); };

    // is it worth making this noexcept?
    void run();

private:
    // might want to write custom ctor to reserver... can get to this during perf
    std::priority_queue<Event, std::vector<Event>, std::greater<Event>> _queue;
    std::function<void(const Event&)> _handler = nullptr;
};
