#include <vector>
#include <format>
#include <list>
#include <map>
#include <unordered_map>
#include <numeric>

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
    void Fill(Quantity quantity) {
        if (quantity > GetRemainingQuantity()) {
            throw std::logic_error(std::format("Order ({}) cannot be filled for more than it's remaining quantity.", GetOrderID()));
        }

        _remainingQuantity -= quantity;
    }
    
private:
    void _setPrice(Price price);
    void _setQuantity(Quantity quantity);
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
    OrderModify(OrderID orderID, Price price, Quantity quantity)
    : _orderID { orderID }
    , _price { price }
    , _quantity { quantity }
    { }

    OrderID GetOrderID() const { return _orderID; }
    Price GetPrice() const { return _price; }
    Quantity GetQuantity() const { return _quantity; }

    OrderPointer ToOrderPointer(OrderType type) const {
        return std::make_shared<Order>(type, GetOrderID(), GetPrice(), GetQuantity());
    };

private:
    Quantity _quantity;
    Price _price;
    OrderID _orderID;
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
    const TradeInfo& getAskTrade() const { return _askTrade; }

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


    bool CanMatch(Side side, Price price) const {
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


    Trades MatchOrders() {
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
                    bids.pop_front();
                    _orders.erase(bid->GetOrderID());
                }

                if (ask->IsFilled()) {
                    asks.pop_front();
                    _orders.erase(ask->GetOrderID());
                }

                if (bids.empty()) {
                    _bids.erase(bidPrice);
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

public:
    void CancelOrder(OrderID orderID) {
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

    }
    Trades AddOrder(OrderPointer order) {
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


    Trades MatchOrder(OrderModify order) {
        if (!_orders.contains(order.GetOrderID())) {
            return { };
        }

        const auto& [existingOrder, _] = _orders.at(order.GetOrderID());
        CancelOrder(order.GetOrderID());
        return AddOrder(order.ToOrderPointer(existingOrder->GetOrderType()));
    }

    std::size_t Size() const { return _orders.size(); }

    OrderbookLevelInfos GetOrderInfos() {
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
};
