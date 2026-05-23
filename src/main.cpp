#include <stdio.h>
#include <iostream>
#include "event_loop.h"
#include "data_feed.h"
#include <print>


int main(int argc, char * argv[]) {
    std::string path = "./test_data.csv";

    EventLoop event_loop{ };
    DataFeed data_feed{path};
    for (auto& event : data_feed) {
        event_loop.AddEvent(event);
        std::print("{}: {}\n", event.instrument, event.timestamp);
    }
}
