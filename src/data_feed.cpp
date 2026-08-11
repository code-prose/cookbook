#include "types.h"
#include "data_feed.h"
#include "parsing.h"
#include <chrono>
#include <charconv>
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
    std::string_view sv{};
    if (!mfile_.getline(sv)) {
        throw std::runtime_error("EOF");
        // can I do this without throwing?
    }
        
    // if I don't allocate here then the lifetime is tied to mmap being open
    std::vector<std::string_view> parts{MappedFile::split_sv(sv)};

    // 0: timestamp, 1: id, 2: type, 3: trade_price, 4: trade_quantity, 5: side, 6: ask_price, 7: ask_quantity, 8: bid_price, 9: bid_quantity can I use a better data format? can I mock a data stream?
    std::int64_t since_epoch{};
    std::from_chars(parts[0].begin(), parts[0].end(), since_epoch);
    Time timestamp{ std::chrono::nanoseconds(since_epoch) };
    std::string instrument{parts[1]};

    // getting fucked by branch prediction in multiple places
    // is there a better way to parse this?
    // could I change the format?
    // is there a way to get this information without string comparison?
    // what do I know about this data?
    // I know that I always have the same number of commas
    // I know that the first letter after the first comma is always a T or a Q
    // what it be faster to do a SIMD pass over first to determine the type?
    // ... does this even work? I would need a different # of something to determine
    // do these have the same number of elements? could I change the data format and then precompute and do a cmov?
    if (parts[2] == "trade") [[unlikely]] {
        // parsing logic
        int quant{};
        std::from_chars(parts[4].begin(), parts[4].end(), quant);
        if (quant < 0) throw std::runtime_error("Quantity < 0"); // kinda gross.. tom said to think about using less exceptions
        Quantity quantity{ static_cast<std::uint32_t>(quant)};
        Side side;
        if (parts[5] == "buy") {
            side = Side::Buy;
        } else {
            side = Side::Sell;
        }

        std::uint64_t price{};
        CustomParsing::int_from_float_chars(parts[3].begin(), parts[3].end(), price);
        TradeEvent tradeEvent{ price, side, quantity };
        // no nrvo or rvo because I return based on branch
        return Event{ timestamp, std::move(instrument), tradeEvent };
    } else [[likely]] {
        // is there a cleaner way to do this?
        // this feels expensive
        int bQuant{};
        int aQuant{};
        std::from_chars(parts[9].begin(), parts[9].end(), bQuant);
        std::from_chars(parts[7].begin(), parts[7].end(), aQuant);
        // am I guarding against an impossibility? I know I am for my generated events
        if (bQuant < 0) throw std::runtime_error("Bid quantity < 0");
        if (aQuant < 0) throw std::runtime_error("Ask quantity < 0");
        Quantity buyQuantity{ static_cast<std::uint32_t>(bQuant)};
        Quantity askQuantity{ static_cast<std::uint32_t>(aQuant)};
        std::uint64_t bidPrice{};
        std::uint64_t askPrice{};
        CustomParsing::int_from_float_chars(parts[8].begin(), parts[8].end(), bidPrice);
        CustomParsing::int_from_float_chars(parts[6].begin(), parts[6].end(), askPrice);
        QuoteEvent quoteEvent{ bidPrice, buyQuantity, askPrice, askQuantity};
        return Event{ timestamp, std::move(instrument), quoteEvent};
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
