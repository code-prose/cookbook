#include "types.h"
#include "event_loop.h"
#include <stdexcept>

void EventLoop::run() {
    if (!_handler) throw std::runtime_error("No registered event handler");

    while(!_queue.empty()) {
        Event event = _queue.top();
        _handler(event);
        _queue.pop();
    }
}


