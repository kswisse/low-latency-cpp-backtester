#pragma once
#include "StrategyInterface.hpp"
#include <random>

class SimpleMarketMaker : public StrategyInterface {
public:
    SimpleMarketMaker(double spread_bps = 10.0, uint64_t order_size = 100);

    void on_market_data(const MarketDataEvent& event, const OrderBook& order_book) override;
    void on_fill(const Fill& fill) override;

private:
    double spread_bps_;
    uint64_t order_size_;
    uint64_t last_event_count_;
    double position_;
    uint64_t next_order_id_;
};
