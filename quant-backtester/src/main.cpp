#include <iostream>
#include <random>
#include <chrono>
#include "OrderBook.hpp"
#include "MarketDataEngine.hpp"
#include "ExecutionSimulator.hpp"
#include "PerformanceMetrics.hpp"
#include "StrategyInterface.hpp"
#include "Backtester.hpp"
#include "SimpleMarketMaker.hpp"

std::vector<MarketDataEvent> generate_mock_events(size_t num_events, double start_price = 100.0) {
    std::vector<MarketDataEvent> events;
    events.reserve(num_events);

    std::mt19937 rng(42);
    std::normal_distribution<double> price_dist(0.0, 0.1);
    std::uniform_int_distribution<uint64_t> quantity_dist(10, 1000);

    double current_price = start_price;
    uint64_t timestamp = 0;

    for (size_t i = 0; i < num_events; ++i) {
        EventType type = (i % 2 == 0) ? EventType::BID_UPDATE : EventType::ASK_UPDATE;
        double price_change = price_dist(rng);
        current_price += price_change;

        double price = type == EventType::BID_UPDATE ? current_price - 0.1 : current_price + 0.1;
        uint64_t quantity = quantity_dist(rng);

        events.push_back({type, price, quantity, timestamp});
        timestamp += 1000;
    }

    return events;
}

int main() {
    const size_t NUM_EVENTS = 1000000;
    std::cout << "Generating " << NUM_EVENTS << " mock events..." << std::endl;

    auto events = generate_mock_events(NUM_EVENTS);

    std::cout << "Running backtest..." << std::endl;
    Backtester backtester;
    backtester.set_strategy(std::make_unique<SimpleMarketMaker>(10.0, 100));

    for (const auto& event : events) {
        backtester.add_market_event(event);
    }

    auto start = std::chrono::high_resolution_clock::now();
    backtester.run();
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    double events_per_second = (double)NUM_EVENTS / (duration.count() / 1000.0);

    const auto& metrics = backtester.get_performance_metrics();

    std::cout << "\n=== PERFORMANCE METRICS ===" << std::endl;
    std::cout << "Time taken: " << duration.count() << " ms" << std::endl;
    std::cout << "Events per second: " << events_per_second << std::endl;
    std::cout << "\n=== TRADING METRICS ===" << std::endl;
    std::cout << "Total fills: " << backtester.get_fills().size() << std::endl;

    return 0;
}
