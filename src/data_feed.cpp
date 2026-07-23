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
    // stopping string allocation every single loop iteration
    if (!std::getline(_fs, _line)) {
        throw std::runtime_error("Could not parse line from input");
    }
    std::stringstream ss(_line);

    // string allocation every single comma, could use std::string_view
    std::string item;
    std::vector<std::string> parts;
    while (std::getline(ss, item, ',')) {
        parts.push_back(item);
    }

    // 0: timestamp, 1: id, 2: type, 3: trade_price, 4: trade_quantity, 5: side, 6: ask_price, 7: ask_quantity, 8: bid_price, 9: bid_quantity
    // can I use a better data format? can I mock a data stream?
    std::int64_t since_epoch = std::stoll(parts[0]);
    Time timestamp{ std::chrono::nanoseconds(since_epoch) };
    std::string instrument = parts[1];

    // getting fucked by branch prediction in multiple places
    // is there a better way to parse this?
    // could I change the format?
    // is there a way to get this information without string comparison?
    // what do I know about this data?
    // I know that I always have the same number of commas
    // I TECHNICALLY know that the ticker is always TEST-ID
    // but this is not a good thing to work off of...
    // I know that the first letter after the first comma is always a T or a Q
    // what it be faster to do a SIMD pass over first to determine the type?
    // ... does this even work? I would need a different # of something to determine
    if (parts[2] == "trade") [[unlikely]] {
        // parsing logic
        int quant = std::stoi(parts[4]);
        if (quant < 0) throw std::runtime_error("Quantity < 0"); // kinda gross.. tom said to think about using less exceptions
        Quantity quantity{ static_cast<std::uint32_t>(quant)};
        Side side;
        if (parts[5] == "buy") {
            side = Side::Buy;
        } else {
            side = Side::Sell;
        }

        TradeEvent tradeEvent{ std::stof(parts[3]), side, quantity };
        // no nrvo or rvo because I return based on branch
        return Event{ timestamp, instrument, tradeEvent };
    } else [[likely]] {
        // is there a cleaner way to do this?
        // this feels expensive
        int bQuant = std::stoi(parts[9]);
        int aQuant = std::stoi(parts[7]);
        // am I guarding against an impossibility? I know I am for my generated events
        if (bQuant < 0) throw std::runtime_error("Bid quantity < 0");
        if (aQuant < 0) throw std::runtime_error("Ask quantity < 0");
        Quantity buyQuantity{ static_cast<std::uint32_t>(bQuant)};
        Quantity askQuantity{ static_cast<std::uint32_t>(aQuant)};
        QuoteEvent quoteEvent{ std::stof(parts[8]), buyQuantity, std::stof(parts[6]), askQuantity};
        return Event{ timestamp, instrument, quoteEvent};
    }

}

DataFeed::Iterator DataFeed::begin() {
    try {
        Event first_event = ParseEvent();
        DataFeed::Iterator iter = {this, first_event, false};
        return iter;
    // should I get rid of this?
    } catch (const std::runtime_error& e) {
        return end();
    }
};

DataFeed::Iterator DataFeed::end() {
    DataFeed::Iterator iter = {this, Event{ }, true}; 
    return iter;
};
