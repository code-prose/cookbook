#include "mapped.h"
#include "data_feed.h"
#include <chrono>
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
    }
    auto end = std::chrono::steady_clock::now();

    double elapsed = std::chrono::duration<double>(end - start).count();
    std::cout << "events: " << count << "\n";
    std::cout << "elapsed: " << elapsed << "s\n";
    std::cout << "ns/event: " << (elapsed * 1e9 / static_cast<double>(count)) << "\n";
    // keeps the compiler from optimizing the whole loop away
    std::cout << "checksum: " << checksum << "\n";
    return 0;
}
