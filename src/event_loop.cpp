#include "types.h"
#include "event_loop.h"
#include <print>
#include <cstdio>

void EventLoop::run() {
    if (_handler == nullptr) {
        std::print(stderr, "Please register a fn");
        exit(127);
    }

    while(!_queue.empty()) {
        Event event = _queue.top();
        _handler(event);
        _queue.pop();
    }
}


