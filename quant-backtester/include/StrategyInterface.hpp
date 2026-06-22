#pragma once
#include "OrderBook.hpp"
#include "MarketDataEvent.hpp"
#include "Order.hpp"
#include <vector>

using OrderSubmitCallback = std::function<void(const Order&)>;

class StrategyInterface {
public:
    virtual ~StrategyInterface() = default;

    virtual void on_market_data(const MarketDataEvent& event, const OrderBook& order_book) = 0;
    virtual void on_fill(const Fill& fill) = 0;

    void set_order_submit_callback(OrderSubmitCallback callback) {
        order_submit_callback_ = std::move(callback);
    }

protected:
    void submit_order(const Order& order) {
        if (order_submit_callback_) {
            order_submit_callback_(order);
        }
    }

private:
    OrderSubmitCallback order_submit_callback_;
};
