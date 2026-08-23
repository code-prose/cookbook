#include "mapped.h"
#include "data_feed.h"
#include <chrono>
#include <cassert>
#include <iostream>

int main() {
    std::string path = "./test_data.csv";
    MappedFile mfile{path};
    DataFeed data_feed{mfile};

    std::size_t count = 0;
    std::int64_t checksum = 0;

    auto start = std::chrono::steady_clock::now();
    for (auto& event : data_feed) {
        ++count;
        checksum += event.timestamp.time_since_epoch().count();
        std::visit(
            [&](auto &payload) {
              using T = std::decay_t<decltype(payload)>;
              if constexpr (std::is_same_v<T, TradeEvent>) {
                auto ev = static_cast<TradeEvent>(payload);
                checksum += ev.price;
              } else if constexpr (std::is_same_v<T, QuoteEvent>) {
                auto ev = static_cast<QuoteEvent>(payload);
                checksum += ev.ask_price;
              }
            },
            event.payload);
    }
    auto end = std::chrono::steady_clock::now();

    double elapsed = std::chrono::duration<double>(end - start).count();
    std::cout << "events: " << count << "\n";
    std::cout << "elapsed: " << elapsed << "s\n";
    std::cout << "ns/event: " << (elapsed * 1e9 / static_cast<double>(count)) << "\n";

    // keeps the compiler from optimizing the whole loop away
    std::cout << "checksum: " << checksum << "\n";
    assert(checksum == 8198660791791185920);
    return 0;
}
