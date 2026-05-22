#pragma once

#include <numeric>
#include <vector>

using Price = std::int32_t;
using Quantity = std::uint32_t;
using OrderID = std::uint64_t;

enum class OrderType {
    FillOrKill,
    MarketOrder,
    LimitOrder,
    StopLossOrder,
    BuyStopOrder,
};

enum class Side {
    Buy,
    Sell
};

struct LevelInfo {
    Price price_;
    Quantity quantity_;
};

using LevelInfos = std::vector<LevelInfo>;
