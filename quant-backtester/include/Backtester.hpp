#pragma once
#include "OrderBook.hpp"
#include "MarketDataEngine.hpp"
#include "ExecutionSimulator.hpp"
#include "StrategyInterface.hpp"
#include "PerformanceMetrics.hpp"
#include <memory>

class Backtester {
public:
    Backtester();

    void set_strategy(std::unique_ptr<StrategyInterface> strategy);
    void add_market_event(const MarketDataEvent& event);
    void run();
    void reset();

    const PerformanceMetrics& get_performance_metrics() const { return metrics_; }
    const OrderBook& get_order_book() const { return order_book_; }
    const std::vector<Fill>& get_fills() const { return simulator_.get_fills(); }

private:
    OrderBook order_book_;
    MarketDataEngine data_engine_;
    ExecutionSimulator simulator_;
    std::unique_ptr<StrategyInterface> strategy_;
    PerformanceMetrics metrics_;
    double current_equity_;
};
