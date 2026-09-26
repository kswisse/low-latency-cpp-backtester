#pragma once

// Shared helpers for the unit-test suite: market-event builders, the
// ExecutionSimulator default parameters, a canonical two-sided book, an
// order-capture helper, and test-side oracle formulas for expected fills/fees.

#include "ExecutionSimulator.hpp"
#include "MarketDataEngine.hpp"
#include "OrderBook.hpp"
#include "StrategyInterface.hpp"

#include <cstdint>
#include <vector>

// --- market event builders -------------------------------------------------

inline MarketDataEvent bid(double price, uint64_t quantity, uint64_t timestamp) {
    return MarketDataEvent{EventType::BID_UPDATE, price, quantity, timestamp};
}

inline MarketDataEvent ask(double price, uint64_t quantity, uint64_t timestamp) {
    return MarketDataEvent{EventType::ASK_UPDATE, price, quantity, timestamp};
}

inline MarketDataEvent trade(double price, uint64_t quantity, uint64_t timestamp) {
    return MarketDataEvent{EventType::TRADE, price, quantity, timestamp};
}

// --- ExecutionSimulator default arguments (include/ExecutionSimulator.hpp:17)
// If the production defaults change and these are not updated, every assertion
// built on them fails instead of silently drifting along.
inline constexpr double kFeePerShare = 0.001;
inline constexpr uint64_t kLatencyNs = 100000;
inline constexpr double kSlippageBps = 0.5;
inline constexpr double kBpsScale = 10000.0;

// --- fixtures ---------------------------------------------------------------

// Two-sided book shared by the simulator and market-maker tests.
inline OrderBook make_two_sided_book() {
    OrderBook book;
    book.update_bid(100.0, 10);
    book.update_ask(100.2, 20);
    return book;
}

// Captures every order a strategy submits into sink.
inline void record_orders(StrategyInterface& strategy, std::vector<Order>& sink) {
    strategy.set_order_submit_callback([&sink](const Order& order) { sink.push_back(order); });
}

// --- test-side oracles ------------------------------------------------------
// Formulas copied verbatim from the original assertion sites. They deliberately
// do NOT call production code - deriving expectations from the code under test
// would make the assertions tautological.

inline double expected_fill_price(double price, OrderSide side, double slippage_bps) {
    if (side == OrderSide::BUY) {
        return price * (1.0 + slippage_bps / kBpsScale);
    }
    return price / (1.0 + slippage_bps / kBpsScale);
}

inline double expected_fees(uint64_t quantity, double fee_per_share) {
    return fee_per_share * static_cast<double>(quantity);
}
