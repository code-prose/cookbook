#include "types.h"
#include "data_feed.h"
#include <chrono>
#include <sstream>

const Event& DataFeed::Iterator::operator*() const {
    return _current;
};

DataFeed::Iterator& DataFeed::Iterator::operator++() {
    std::string line;
    std::getline(_feed->_fs, line);
    std::stringstream ss(line);

    std::string item;
    std::vector<std::string> parts;
    while (std::getline(ss, item, ',')) {
        parts.push_back(item);
    }

    // 0: timestamp, 1: id, 2: type, 3: trade_price, 4: trade_quantity, 5: side, 6: ask_price, 7: ask_quantity, 8: bid_price, 9: bid_quantity
    std::int64_t since_epoch = std::stoll(parts[0]);
    Time timestamp{ std::chrono::nanoseconds(since_epoch) };
    std::string instrument = parts[1];
    Event event;

    // getting fucked by branch prediction in multiple places
    if (parts[2] == "trade") {
        // parsing logic
        std::int16_t quant = std::stoi(parts[3]);
        if (quant < 0) throw std::runtime_error("Quantity < 0");
        Quantity quantity{quant};
        Side side;
        if (parts[4] == "buy") {
            side = Side::Buy;
        } else {
            side = Side::Sell;
        }

        TradeEvent tradeEvent{ std::stoi(parts[3]), side, quantity };
        event = Event{ timestamp, instrument, tradeEvent };

    } else {
        std::int16_t bQuant = std::stoi(parts[9]);
        std::int16_t aQuant = std::stoi(parts[7]);
        if (bQuant < 0) throw std::runtime_error("Bid quantity < 0");
        if (aQuant < 0) throw std::runtime_error("Ask quantity < 0");
        Quantity buyQuantity{bQuant};
        Quantity askQuantity{aQuant};
        QuoteEvent quoteEvent{ };
        event = Event{ timestamp, instrument, quoteEvent};

    }

    

};

DataFeed::Iterator DataFeed::begin() {

};

DataFeed::Iterator DataFeed::end() {

};
