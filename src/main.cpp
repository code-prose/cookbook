#include "event_loop.h"
#include "data_feed.h"
#include "strategy.h"
#include <print>


int main(int argc, char * argv[]) {
    std::string path = "./test_data.csv";

    EventLoop event_loop{ };
    DataFeed data_feed{path};
    OrderBook ob{ };
    NaiveStrategy ns{ob, 50'000.0f, 0.1f};
    event_loop.AddHandler([&ns](const Event& e) { ns.execute(e); });

    for (auto& event : data_feed) {
        event_loop.AddEvent(event);
        std::print("{}: {}\n", event.instrument, event.timestamp);
    }

    event_loop.run();
}
