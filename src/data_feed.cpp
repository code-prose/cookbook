#include "types.h"
#include "data_feed.h"
#include <chrono>
#include <sstream>
#include <stdexcept>

const Event& DataFeed::Iterator::operator*() const {
    return _current;
};

DataFeed::Iterator& DataFeed::Iterator::operator++() {
    try {
        _current = _feed->ParseEvent();
    } catch (const std::runtime_error&){
        _done = true;
    }
    return *this;
};

bool DataFeed::Iterator::operator!=(const Iterator& other) const {
    return _done != other._done;
}


Event DataFeed::ParseEvent() {
    std::string line;
    if (!std::getline(_fs, line)) {
        throw std::runtime_error("Could not parse line from input");
    }
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

    // getting fucked by branch prediction in multiple places
    if (parts[2] == "trade") {
        // parsing logic
        int quant = std::stoi(parts[4]);
        if (quant < 0) throw std::runtime_error("Quantity < 0");
        Quantity quantity{ static_cast<std::uint32_t>(quant)};
        Side side;
        if (parts[5] == "buy") {
            side = Side::Buy;
        } else {
            side = Side::Sell;
        }

        TradeEvent tradeEvent{ std::stoi(parts[3]), side, quantity };
        return Event{ timestamp, instrument, tradeEvent };


    } else {
        int bQuant = std::stoi(parts[9]);
        int aQuant = std::stoi(parts[7]);
        if (bQuant < 0) throw std::runtime_error("Bid quantity < 0");
        if (aQuant < 0) throw std::runtime_error("Ask quantity < 0");
        Quantity buyQuantity{ static_cast<std::uint32_t>(bQuant)};
        Quantity askQuantity{ static_cast<std::uint32_t>(aQuant)};
        QuoteEvent quoteEvent{ std::stoi(parts[8]), buyQuantity, std::stoi(parts[6]), askQuantity};
        return Event{ timestamp, instrument, quoteEvent};

    }

}

DataFeed::Iterator DataFeed::begin() {
    Event first_event = ParseEvent();
    DataFeed::Iterator iter = {this, first_event, false};
    return iter;
};

DataFeed::Iterator DataFeed::end() {
    DataFeed::Iterator iter = {this, Event{ }, true}; 
    return iter;
};
