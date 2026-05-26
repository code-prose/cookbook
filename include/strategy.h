#pragma once

#include "order.h"
#include "types.h"

class NaiveStrategy {
public:
    NaiveStrategy(OrderBook& ob, StrategyBalance starting_balance, float alpha)
    : _ob { ob }
    , _position { 0 }
    , _balance { starting_balance }
    , _alpha { alpha }
    , _ema { 0.0f }
    {}
    void execute(const Event& event); 

private:
    void OnTrade(const TradeEvent& event);
    void OnQuote(const QuoteEvent& event);

    OrderBook& _ob;
    StrategyQuantity _position;
    StrategyBalance _balance;
    float _ema;
    float _alpha;
    OrderID _nextOrder { 1 };
    Quantity _naiveQuantity { 10 };
};
