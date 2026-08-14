#include "types.h"
#include "data_feed.h"
#include "parsing.h"
#include <chrono>
#include <stdexcept>
#include <array>



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
    // 0: timestamp, 1: id, 2: type, 3: trade_price, 4: trade_quantity, 5: side, 6: ask_price, 7: ask_quantity, 8: bid_price, 9: bid_quantity
    std::string_view sv{};
    if (!mfile_.getline(sv)) {
        throw std::runtime_error("EOF");
        // can I do this without throwing?
    }
        
    std::array<std::string_view, 10> parts{MappedFile::split_sv(sv)};
    constexpr std::uint64_t chunk_1 = 1e16;
    constexpr std::uint64_t chunk_2 = 1e8;

    std::uint64_t since_epoch{0};
    auto ptr = parts[0].data();
    for (auto i{0uz}; i < 3; i++) {
        since_epoch = since_epoch * 10 + static_cast<std::uint64_t>(*(ptr + i) - '0');
    }

    {
        using namespace CustomParsing;
        since_epoch = since_epoch * chunk_1 + swar_eight_digits<std::uint64_t>(parts[0].begin() + 3) * chunk_2 + swar_eight_digits<std::uint64_t>(parts[0].begin() + 11);
    }

    Time timestamp{ std::chrono::nanoseconds(since_epoch) };
    std::array<char, 5> instrument{};
    std::memcpy(&instrument, parts[1].data(), parts[1].size());

    // the branch predictor is actually super accurate here and precompute with cmov would just add unnecessary compute cost
    // always check first...
    // data:
    // Performance counter stats for './build/microbench' (10 runs):
    //
    //             86.21 msec task-clock:u                                                            ( +-  1.67% )
    //       379,503,799      cycles:u                                                                ( +-  0.67% )  (82.79%)
    //     1,346,215,416      instructions:u                                                          ( +-  0.07% )  (82.71%)
    //         4,780,215      cache-references:u                                                      ( +-  0.09% )  (83.36%)
    //            34,149      cache-misses:u                                                          ( +-  2.26% )  (84.04%)
    //       208,179,326      branches:u                                                              ( +-  0.08% )  (83.97%)
    //         1,714,586      branch-misses:u                                                         ( +-  0.10% )  (83.12%)
    //
    //       0.087101473 +- 0.001449441 seconds time elapsed  ( +-  1.66% )
    //  1,714,586 / 208,179,326 ~= .8% misprediction
    if (parts[2] == "trade") [[unlikely]] {
        // parsing logic
        int quant{};
        {
            using namespace CustomParsing;
            quant = left_pad_and_swar<int>(parts[4]);
        }
        if (quant < 0) throw std::runtime_error("Quantity < 0"); // kinda gross.. tom said to think about using less exceptions
        Quantity quantity{static_cast<std::uint32_t>(quant)};
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
        {
            using namespace CustomParsing;
            bQuant = left_pad_and_swar<int>(parts[9]);
            aQuant = left_pad_and_swar<int>(parts[7]);
        }
        // std::from_chars(parts[9].begin(), parts[9].end(), bQuant);
        // std::from_chars(parts[7].begin(), parts[7].end(), aQuant);
        // am I guarding against an impossibility? I know I am for my generated events
        // I will almost never guess these wrong but maybe I should handle different than throwing an err
        if (bQuant < 0) throw std::runtime_error("Bid quantity < 0");
        if (aQuant < 0) throw std::runtime_error("Ask quantity < 0");
        Quantity buyQuantity{static_cast<std::uint32_t>(bQuant)};
        Quantity askQuantity{static_cast<std::uint32_t>(aQuant)};
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
