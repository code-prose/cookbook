#include <vector>
#include <format>

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

    Price GetPrice() const { return _price; };
    Quantity GetQuantity() const { return _quantity; };
    Side GetSide() const { return _side; };
    OrderType GetOrderType() const { return _orderType; };
    OrderID GetOrderID() const { return _orderID; };
    Quantity GetInitialQuantity() const { return _initialQuantity; };
    Quantity GetRemainingQuantity() const { return _remainingQuantity; };
    Quantity GetFilledQuantity() const { return _initialQuantity - _remainingQuantity; };

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
