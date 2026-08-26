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

    // timestamp parsing
    auto since_epoch = CustomParsing::parse_timestamp_beginning(parts[0]);
    {
        using namespace CustomParsing;
        auto epoch_ptr = parts[0].begin();
        since_epoch = since_epoch * chunk_1 + swar_eight_digits<std::uint64_t>(epoch_ptr + 3) * chunk_2 + swar_eight_digits<std::uint64_t>(epoch_ptr + 11);
    }

    Time timestamp{ std::chrono::nanoseconds(since_epoch) };
    std::array<char, 5> instrument{};
    std::memcpy(&instrument, parts[1].data(), parts[1].size());

    if (parts[2] == QuoteType::Trade) [[unlikely]] {
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
    } else if (parts[2] == QuoteType::Quote) [[likely]] {
        int bQuant{};
        int aQuant{};
        {
            using namespace CustomParsing;
            bQuant = left_pad_and_swar<int>(parts[9]);
            aQuant = left_pad_and_swar<int>(parts[7]);
        }
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

    // Impossible to reach, probably a better way to handle this though
    std::terminate();
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
