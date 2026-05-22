#pragma once

#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include "types.h"

class OrderbookLevelInfos {
public:
    OrderbookLevelInfos(const LevelInfos& bids, const LevelInfos& asks) : _bids { bids } , _asks{ asks } {}

    const LevelInfos& GetBids() const { return _bids; }
    const LevelInfos& GetAsks() const { return _asks; }

private:
    LevelInfos _bids;
    LevelInfos _asks;

};

class Order {
public:
    Order(Price price, Quantity quantity, Side side, OrderType orderType, OrderID orderID)
    : _price { price }
    , _quantity { quantity }
    , _side { side }
    , _orderType { orderType }
    , _orderID { orderID }
    , _initialQuantity { quantity }
    , _remainingQuantity { quantity }
    { }

    Price GetPrice() const { return _price; }
    Quantity GetQuantity() const { return _quantity; }
    Side GetSide() const { return _side; }
    OrderType GetOrderType() const { return _orderType; }
    OrderID GetOrderID() const { return _orderID; }
    Quantity GetInitialQuantity() const { return _initialQuantity; }
    Quantity GetRemainingQuantity() const { return _remainingQuantity; }
    Quantity GetFilledQuantity() const { return _initialQuantity - _remainingQuantity; }

    bool IsFilled() const { return GetRemainingQuantity() == 0; }
    void Fill(Quantity quantity);
    
private:
    void _setPrice(Price price) { _price = price; };
    void _setQuantity(Quantity quantity) { _quantity = quantity; };
    Price _price;
    Quantity _quantity;
    OrderType _orderType;
    OrderID _orderID;
    Side _side;
    Quantity _initialQuantity;
    Quantity _remainingQuantity;
};

using OrderPointer = std::shared_ptr<Order>;
using OrderPointers = std::list<OrderPointer>;


class OrderModify {
public:
    OrderModify(OrderID orderID, Price price, Quantity quantity, Side side)
    : _orderID { orderID }
    , _price { price }
    , _quantity { quantity }
    , _side { side }
    { }

    OrderID GetOrderID() const { return _orderID; }
    Price GetPrice() const { return _price; }
    Quantity GetQuantity() const { return _quantity; }
    Side GetSide() const { return _side; }

    OrderPointer ToOrderPointer(OrderType type) const {
        return std::make_shared<Order>(GetPrice(), GetQuantity(), GetSide(), type, GetOrderID());
    };

private:
    Quantity _quantity;
    Price _price;
    OrderID _orderID;
    Side _side;
};

struct TradeInfo {
    OrderID _orderID;
    Price _price;
    Quantity _quantity;
};


class Trade {
public:
    Trade(const TradeInfo& bidTrade, const TradeInfo& askTrade)
    : _bidTrade { bidTrade }
    , _askTrade { askTrade }
    { }

    const TradeInfo& GetBidTrade() const { return _bidTrade; }
    const TradeInfo& GetAskTrade() const { return _askTrade; }

private:
    TradeInfo _bidTrade;
    TradeInfo _askTrade;
};


using Trades = std::vector<Trade>;

class OrderBook {
private:

    struct OrderEntry {
        OrderPointer _order { nullptr };
        OrderPointers::iterator _location;
    };

    std::map<Price, OrderPointers, std::greater<Price>> _bids;
    std::map<Price, OrderPointers, std::less<Price>> _asks;
    std::unordered_map<OrderID, OrderEntry> _orders;


    bool CanMatch(Side side, Price price) const;


    Trades MatchOrders();

public:
    void CancelOrder(OrderID orderID);
    Trades AddOrder(OrderPointer order);

    Trades MatchOrder(OrderModify order);
    std::size_t Size() const { return _orders.size(); }

    OrderbookLevelInfos GetOrderInfos();
};
