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
    // 0: timestamp, 1: id, 2: type, 3: trade_price, 4: trade_quantity, 5: side, 6: ask_price, 7: ask_quantity, 8: bid_price, 9: bid_quantity can I use a better data format? can I mock a data stream?
    std::string_view sv{};
    if (!mfile_.getline(sv)) {
        throw std::runtime_error("EOF");
        // can I do this without throwing?
    }
        
    std::vector<std::string_view> parts{MappedFile::split_sv(sv)};

    // move to swar
    std::uint64_t since_epoch{0};
    auto ptr = parts[0].data();
    for (auto i{0uz}; i < 19; i++) {
        since_epoch = since_epoch * 10 + static_cast<std::uint64_t>(*(ptr + i) - '0');
    }
    Time timestamp{ std::chrono::nanoseconds(since_epoch) };
    std::string instrument{parts[1]};

    // the branch predictor is actually super accurate here and precompute with cmov would just add unnecessary latency
    // always check first...
    // data:
    // events: 2000000
    // elapsed: 0.170286s
    // ns/event: 85.1432
    // checksum: 8198660791791185920
    //
    //  Performance counter stats for './build/microbench':
    //
    //             170.99 msec task-clock:u
    //        736,144,717      cycles:u                                                                (83.00%)
    //      2,632,964,826      instructions:u                                                          (83.04%)
    //          4,926,044      cache-references:u                                                      (83.47%)
    //             40,373      cache-misses:u                                                          (83.62%)
    //        625,292,269      branches:u                                                              (83.65%)
    //          1,589,856      branch-misses:u                                                         (83.21%)
    //
    //        0.171796613 seconds time elapsed
    //
    //        0.169223000 seconds user
    //        0.000996000 seconds sys
    //  1,589,856 / 625,292,269 ~= .25% misprediction
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
        return Event{ timestamp, std::move(instrument), tradeEvent };
    } else [[likely]] {
        int bQuant{};
        int aQuant{};
        std::from_chars(parts[9].begin(), parts[9].end(), bQuant);
        std::from_chars(parts[7].begin(), parts[7].end(), aQuant);
        // am I guarding against an impossibility? I know I am for my generated events
        // I will almost never guess these wrong but maybe I should handle different than throwing an err
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
    } catch (const std::runtime_error& e) {
        return end();
    }
};

DataFeed::Iterator DataFeed::end() {
    DataFeed::Iterator iter = {this, Event{ }, true}; 
    return iter;
};
