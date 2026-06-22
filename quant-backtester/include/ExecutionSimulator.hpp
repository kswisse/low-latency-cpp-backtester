#pragma once
#include "Order.hpp"
#include "OrderBook.hpp"
#include <vector>
#include <cstdint>

struct Fill {
    uint64_t order_id;
    double fill_price;
    uint64_t fill_quantity;
    uint64_t timestamp;
    double fees;
};

class ExecutionSimulator {
public:
    ExecutionSimulator(double fee_per_share = 0.001, uint64_t latency_ns = 100000, double slippage_bps = 0.5);

    void set_order_book(const OrderBook* order_book);
    void submit_order(const Order& order);
    void process_orders(uint64_t current_timestamp);

    const std::vector<Fill>& get_fills() const { return fills_; }
    void reset();

private:
    const OrderBook* order_book_;
    std::vector<Order> pending_orders_;
    std::vector<Fill> fills_;
    double fee_per_share_;
    uint64_t latency_ns_;
    double slippage_bps_;
    uint64_t next_order_id_;

    void execute_order(const Order& order, uint64_t current_timestamp);
    double calculate_slippage(double price, OrderSide side) const;
};
