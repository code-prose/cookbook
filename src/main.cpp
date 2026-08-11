#include "event_loop.h"
#include "data_feed.h"
#include "strategy.h"
#include "mapped.h"


int main(int argc, char * argv[]) {
    std::string path = "./test_data.csv";
    MappedFile mfile{path};
    DataFeed data_feed{mfile};

    EventLoop event_loop{ };
    OrderBook ob{ };
    NaiveStrategy ns{ob, 50'000.0f, 0.1f};
    event_loop.AddHandler([&ns](const Event& e) { ns.execute(e); });

    for (auto& event : data_feed) {
        event_loop.AddEvent(event);
    }

    // I do not need this for perf testing the data_feed
    event_loop.run();
}
