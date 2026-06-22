#pragma once
#include <cstdint>

enum class OrderSide {
    BUY,
    SELL
};

enum class OrderType {
    MARKET,
    LIMIT
};

struct Order {
    uint64_t order_id;
    OrderSide side;
    OrderType type;
    double price;
    uint64_t quantity;
    uint64_t timestamp;
};
