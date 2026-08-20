#pragma once

#include <chrono>
#include <variant>
#include <vector>

// using this for ease now... let's switch this back to unint32_t and change the parser and then handle the arithmetic for perf
using Price = std::uint64_t;
using Quantity = std::uint32_t;
using OrderID = std::uint64_t;

using StrategyQuantity = std::int32_t;
using StrategyBalance = std::uint64_t;


enum class OrderType {
  FillOrKill,
  MarketOrder,
  LimitOrder,
  StopLossOrder,
  BuyStopOrder,
};


struct QuoteType {
    static constexpr std::string_view Trade = "trade";
    static constexpr std::string_view Quote = "quote";

    QuoteType() = delete; 
};

enum class Side { Buy, Sell };

struct LevelInfo {
  Price price_;
  Quantity quantity_;
};

using LevelInfos = std::vector<LevelInfo>;

using Time = std::chrono::sys_time<std::chrono::nanoseconds>;

struct QuoteEvent {
    Price bid_price;
    Quantity bid_quantity;
    Price ask_price;
    Quantity ask_quantity;
};

struct TradeEvent {
    Price price;
    Side side;
    Quantity quantity;
};

struct Event {
    Time timestamp;
    std::array<char, 5> instrument;
    std::variant<TradeEvent, QuoteEvent> payload;

    bool operator<(const Event& other) const {
        return timestamp < other.timestamp;
    }

    bool operator>(const Event& other) const {
        return timestamp > other.timestamp;
    }

    bool operator==(const Event& other) const {
        return timestamp == other.timestamp;
    }
};
