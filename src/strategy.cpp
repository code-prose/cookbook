#include "strategy.h"
#include <print>

void NaiveStrategy::execute(const Event &event) {
  std::visit(
      [this](auto &payload) {
        using T = std::decay_t<decltype(payload)>;
        if constexpr (std::is_same_v<T, TradeEvent>) {
          OnTrade(payload);
        } else if constexpr (std::is_same_v<T, QuoteEvent>) {
          OnQuote(payload);
        }
      },
      event.payload);
}

void NaiveStrategy::OnTrade(const TradeEvent &event) {
  if (_ema == 0.0f)
    _ema = event.price;
  else
    _ema = _alpha * event.price + (1 - _alpha) * _ema;
  if (event.price < _ema * .9999) {
    auto order = std::make_shared<Order>(event.price, _naiveQuantity, Side::Buy,
                                         OrderType::LimitOrder, _nextOrder);
    _ob.AddOrder(order);
    _balance -= event.price * _naiveQuantity;
    _position += _naiveQuantity;
    _nextOrder += 1;
    std::print("ID {}:, Purchased for {} with quantity {} at ema: {}\n",
                _nextOrder - 1, event.price, _naiveQuantity, _ema);
  } else if (event.price > _ema * 1.00001) {
    auto order =
        std::make_shared<Order>(event.price, _naiveQuantity, Side::Sell,
                                OrderType::LimitOrder, _nextOrder);
    _ob.AddOrder(order);
    _balance += event.price * _naiveQuantity;
    _position -= _naiveQuantity;
    _nextOrder += 1;
    std::print("ID {}:, Sold for {} with quantity {} at ema: {}\n",
                _nextOrder - 1, event.price, _naiveQuantity, _ema);
  }
}

void NaiveStrategy::OnQuote(const QuoteEvent &event) {
  float mid = (event.ask_price + event.bid_price) / 2;
  if (_ema == 0.0f)
    _ema = mid;
  else
    _ema = _alpha * mid + (1 - _alpha) * _ema;
}
