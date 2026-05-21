enum class OrderType {
    FillOrKill,
    MarketOrder,
    LimitOrder,
    StopLossOrder,
    BuyStopOrder
};


using Price = double;
using Quantity = uint64;

class Order {
    Price price;
    Quantity quantity;
    OrderType orderType;

public:
    Order(Price price, Quantity quantity, OrderType orderType) : price(price), quantity(quantity), orderType(orderType) {}

    Price getPrice() noexcept;
    Quantity getQuantity() noexcept;

private:
    void _setPrice(Price price);
    void _setQuantity(Quantity quantity);
}
