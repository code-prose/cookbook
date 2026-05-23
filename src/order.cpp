#include "types.h"
#include "order.h"
#include <format>
#include <memory>
#include <numeric>

void Order::Fill(Quantity quantity) {
    if (quantity > GetRemainingQuantity()) {
        throw std::logic_error(std::format("Order ({}) cannot be filled for more than it's remaining quantity.", GetOrderID()));
    }

    _remainingQuantity -= quantity;
}



bool OrderBook::CanMatch(Side side, Price price) const {
    if (side == Side::Buy) {
        if (_asks.empty()) return false;

        const auto& [bestAsk, _] = *_asks.begin();
        return price >= bestAsk;
    }
    // Side::Sell
    else {
        if (_bids.empty()) return false;

        const auto& [bestBid, _] = *_bids.begin();

        return price <= bestBid;
    }
};


Trades OrderBook::MatchOrders() {
    Trades trades;
    trades.reserve(_orders.size());

    while (true) {
        if (_bids.empty() || _asks.empty()) break;

        auto& [bidPrice, bids] = *_bids.begin();
        auto& [askPrice, asks] = *_asks.begin();

        if (bidPrice < askPrice) break;

        while (bids.size() && asks.size()) {
            auto& bid = bids.front();
            auto& ask = asks.front();

            Quantity quantity = std::min(bid->GetRemainingQuantity(), ask->GetRemainingQuantity());

            bid->Fill(quantity);
            ask->Fill(quantity);


            if (bid->IsFilled()) {
                _orders.erase(bid->GetOrderID());
                bids.pop_front();
            }

            if (ask->IsFilled()) {
                _orders.erase(ask->GetOrderID());
                asks.pop_front();
            }

            if (bids.empty()) {
                _bids.erase(bidPrice);
            }

            if (asks.empty()) {
                _asks.erase(askPrice);
            }

            trades.push_back(Trade{
                TradeInfo{ bid->GetOrderID(), bid->GetPrice(), quantity },
                TradeInfo{ ask->GetOrderID(), ask->GetPrice(), quantity },
            });
        }
    }

    // can abstract this out probably
    if(!_bids.empty()) {
        auto& [_, bids] = *_bids.begin();
        auto& order = bids.front();

        if (order->GetOrderType() == OrderType::FillOrKill) {
            CancelOrder(order->GetOrderID());
        }
    }

    if(!_asks.empty()) {
        auto& [_, asks] = *_asks.begin();
        auto& order = asks.front();

        if (order->GetOrderType() == OrderType::FillOrKill) {
            CancelOrder(order->GetOrderID());
        }
    }

    return trades;
}

void OrderBook::CancelOrder(OrderID orderID) {
    if (!_orders.contains(orderID)) {
        return;
    }

    const auto& [order, orderIterator] = _orders.at(orderID);

    // can abstract this out probably
    if (order->GetSide() == Side::Sell) {
        auto price = order->GetPrice();
        auto& orders = _asks.at(price);
        orders.erase(orderIterator);
        if (orders.empty()) {
            _asks.erase(price);
        }
    } else {
        auto price = order->GetPrice();
        auto& orders = _bids.at(price);
        orders.erase(orderIterator);
        if (orders.empty()) {
            _bids.erase(price);
        }
    }

    _orders.erase(orderID);

}

Trades OrderBook::AddOrder(OrderPointer order) {
    if (_orders.contains(order->GetOrderID())) {
        return { };
    }

    if (order->GetOrderType() == OrderType::FillOrKill && !CanMatch(order->GetSide(), order->GetPrice())) {
        return { };
    }

    OrderPointers::iterator iterator;


    if (order->GetSide() == Side::Buy) {
        auto& orders = _bids[order->GetPrice()];
        orders.push_back(order);
        iterator = std::next(orders.begin(), orders.size() - 1);
    } else {
        auto& orders = _asks[order->GetPrice()];
        orders.push_back(order);
        iterator = std::next(orders.begin(), orders.size() - 1);
    }

    _orders.insert({ order->GetOrderID(), OrderEntry{ order, iterator } });
    return MatchOrders();
}


Trades OrderBook::MatchOrder(OrderModify order) {
    if (!_orders.contains(order.GetOrderID())) {
        return { };
    }

    const auto& [existingOrder, _] = _orders.at(order.GetOrderID());
    CancelOrder(order.GetOrderID());
    return AddOrder(order.ToOrderPointer(existingOrder->GetOrderType()));
}

OrderbookLevelInfos OrderBook::GetOrderInfos() {
    LevelInfos bidInfos, askInfos;

    bidInfos.reserve(_orders.size());
    askInfos.reserve(_orders.size());

    auto CreateLevelInfos = [](Price price, const OrderPointers& orders) {
        return LevelInfo{ price, std::accumulate(orders.begin(), orders.end(), (Quantity)0,
                            [](std::size_t runningSum, const OrderPointer& order)
                             {return runningSum + order->GetRemainingQuantity(); }) };
    };


    for (const auto& [price, orders] : _bids) {
        bidInfos.push_back(CreateLevelInfos(price, orders));
    }

    for (const auto& [price, orders] : _asks) {
        askInfos.push_back(CreateLevelInfos(price, orders));
    }

    return OrderbookLevelInfos{ bidInfos, askInfos };
}
