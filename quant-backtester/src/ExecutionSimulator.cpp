#include "ExecutionSimulator.hpp"

ExecutionSimulator::ExecutionSimulator(double fee_per_share, uint64_t latency_ns, double slippage_bps)
    : order_book_(nullptr), fee_per_share_(fee_per_share), latency_ns_(latency_ns),
      slippage_bps_(slippage_bps), next_order_id_(1) {
    pending_orders_.reserve(10000);
    fills_.reserve(10000);
}

void ExecutionSimulator::set_order_book(const OrderBook* order_book) {
    order_book_ = order_book;
}

void ExecutionSimulator::submit_order(const Order& order) {
    pending_orders_.push_back(order);
}

void ExecutionSimulator::process_orders(uint64_t current_timestamp) {
    for (const auto& order : pending_orders_) {
        uint64_t execution_time = order.timestamp + latency_ns_;
        if (execution_time <= current_timestamp) {
            execute_order(order, current_timestamp);
        }
    }
    pending_orders_.clear();
}

void ExecutionSimulator::reset() {
    pending_orders_.clear();
    fills_.clear();
    next_order_id_ = 1;
}

void ExecutionSimulator::execute_order(const Order& order, uint64_t current_timestamp) {
    if (!order_book_) return;

    double execution_price = 0.0;

    if (order.type == OrderType::MARKET) {
        if (order.side == OrderSide::BUY) {
            execution_price = order_book_->get_best_ask();
        } else {
            execution_price = order_book_->get_best_bid();
        }
    } else {
        execution_price = order.price;
    }

    execution_price = calculate_slippage(execution_price, order.side);
    double fees = fee_per_share_ * order.quantity;

    Fill fill{
        order.order_id,
        execution_price,
        order.quantity,
        current_timestamp,
        fees
    };

    fills_.push_back(fill);
}

double ExecutionSimulator::calculate_slippage(double price, OrderSide side) const {
    double slippage_factor = 1.0 + (slippage_bps_ / 10000.0);
    if (side == OrderSide::BUY) {
        return price * slippage_factor;
    } else {
        return price / slippage_factor;
    }
}
