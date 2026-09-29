#include <doctest/doctest.h>

#include "ExecutionSimulator.hpp"
#include "test_support.hpp"

TEST_CASE("market buy fills at the best ask plus slippage once latency has elapsed") {
    OrderBook book = make_two_sided_book();
    ExecutionSimulator simulator(kFeePerShare, kLatencyNs, kSlippageBps);
    simulator.set_order_book(&book);

    simulator.submit_order(Order{7ull, OrderSide::BUY, OrderType::MARKET, 0.0, 5ull, 1000ull});
    simulator.process_orders(1000ull + kLatencyNs);

    const auto& fills = simulator.get_fills();
    REQUIRE(fills.size() == 1u);

    const Fill& fill = fills[0];
    CHECK(fill.order_id == 7ull);
    CHECK(fill.fill_quantity == 5ull);
    CHECK(fill.timestamp == 1000ull + kLatencyNs);
    CHECK(fill.fill_price == doctest::Approx(expected_fill_price(100.2, OrderSide::BUY, kSlippageBps)));
    CHECK(fill.fees == doctest::Approx(expected_fees(5, kFeePerShare)));
}

TEST_CASE("market sell fills at the best bid minus slippage") {
    OrderBook book = make_two_sided_book();
    ExecutionSimulator simulator(kFeePerShare, kLatencyNs, kSlippageBps);
    simulator.set_order_book(&book);

    simulator.submit_order(Order{8ull, OrderSide::SELL, OrderType::MARKET, 0.0, 3ull, 2000ull});
    simulator.process_orders(2000ull + kLatencyNs);

    const auto& fills = simulator.get_fills();
    REQUIRE(fills.size() == 1u);
    CHECK(fills[0].fill_price == doctest::Approx(expected_fill_price(100.0, OrderSide::SELL, kSlippageBps)));
    CHECK(fills[0].fill_quantity == 3ull);
    CHECK(fills[0].fees == doctest::Approx(expected_fees(3, kFeePerShare)));
}

TEST_CASE("limit orders execute at their own price plus side-dependent slippage") {
    OrderBook book = make_two_sided_book();
    ExecutionSimulator simulator(kFeePerShare, kLatencyNs, kSlippageBps);
    simulator.set_order_book(&book);

    simulator.submit_order(Order{11ull, OrderSide::BUY, OrderType::LIMIT, 100.1, 2ull, 500ull});
    simulator.submit_order(Order{12ull, OrderSide::SELL, OrderType::LIMIT, 99.9, 4ull, 500ull});
    simulator.process_orders(500ull + kLatencyNs);

    const auto& fills = simulator.get_fills();
    REQUIRE(fills.size() == 2u);

    CHECK(fills[0].order_id == 11ull);
    CHECK(fills[0].fill_price == doctest::Approx(expected_fill_price(100.1, OrderSide::BUY, kSlippageBps)));
    CHECK(fills[1].order_id == 12ull);
    CHECK(fills[1].fill_price == doctest::Approx(expected_fill_price(99.9, OrderSide::SELL, kSlippageBps)));
}

TEST_CASE("zero latency, zero fee and zero slippage produce exact book prices") {
    OrderBook book = make_two_sided_book();
    ExecutionSimulator simulator(0.0, 0ull, 0.0);
    simulator.set_order_book(&book);

    simulator.submit_order(Order{1ull, OrderSide::BUY, OrderType::MARKET, 0.0, 10ull, 4242ull});
    simulator.submit_order(Order{2ull, OrderSide::SELL, OrderType::MARKET, 0.0, 6ull, 4242ull});
    simulator.process_orders(4242ull);

    const auto& fills = simulator.get_fills();
    REQUIRE(fills.size() == 2u);
    CHECK(fills[0].fill_price == 100.2);
    CHECK(fills[0].fees == 0.0);
    CHECK(fills[1].fill_price == 100.0);
    CHECK(fills[1].fees == 0.0);
    CHECK(fills[0].timestamp == 4242ull);
}

TEST_CASE("orders are executed in submission order") {
    OrderBook book = make_two_sided_book();
    ExecutionSimulator simulator(kFeePerShare, kLatencyNs, kSlippageBps);
    simulator.set_order_book(&book);

    simulator.submit_order(Order{101ull, OrderSide::BUY, OrderType::LIMIT, 100.0, 1ull, 10ull});
    simulator.submit_order(Order{102ull, OrderSide::BUY, OrderType::LIMIT, 100.1, 1ull, 11ull});
    simulator.submit_order(Order{103ull, OrderSide::SELL, OrderType::LIMIT, 100.2, 1ull, 12ull});
    simulator.process_orders(12ull + kLatencyNs);

    const auto& fills = simulator.get_fills();
    REQUIRE(fills.size() == 3u);
    CHECK(fills[0].order_id == 101ull);
    CHECK(fills[1].order_id == 102ull);
    CHECK(fills[2].order_id == 103ull);
}

TEST_CASE("an order younger than the configured latency never fills and is not retried") {
    OrderBook book = make_two_sided_book();
    ExecutionSimulator simulator(kFeePerShare, kLatencyNs, kSlippageBps);
    simulator.set_order_book(&book);

    simulator.submit_order(Order{5ull, OrderSide::BUY, OrderType::MARKET, 0.0, 5ull, 1000ull});

    simulator.process_orders(1000ull + kLatencyNs - 1ull);
    CHECK(simulator.get_fills().empty());

    simulator.process_orders(1000ull + kLatencyNs + 1000000ull);
    CHECK(simulator.get_fills().empty());
}

TEST_CASE("orders submitted without an order book never fill") {
    ExecutionSimulator simulator(kFeePerShare, kLatencyNs, kSlippageBps);

    simulator.submit_order(Order{9ull, OrderSide::BUY, OrderType::MARKET, 0.0, 5ull, 1000ull});
    simulator.process_orders(1000ull + kLatencyNs);

    CHECK(simulator.get_fills().empty());
}

TEST_CASE("a default-constructed simulator has no book attached") {
    ExecutionSimulator simulator;
    simulator.submit_order(Order{3ull, OrderSide::SELL, OrderType::LIMIT, 99.0, 1ull, 0ull});
    simulator.process_orders(kLatencyNs);

    CHECK(simulator.get_fills().empty());
}

TEST_CASE("fees equal fee-per-share times quantity") {
    constexpr double kFeePerShareAlt = 0.0025;
    constexpr uint64_t kQty = 400;

    OrderBook book = make_two_sided_book();
    ExecutionSimulator simulator(kFeePerShareAlt, kLatencyNs, kSlippageBps);
    simulator.set_order_book(&book);

    simulator.submit_order(Order{1ull, OrderSide::BUY, OrderType::LIMIT, 100.0, kQty, 0ull});
    simulator.process_orders(kLatencyNs);

    REQUIRE(simulator.get_fills().size() == 1u);
    CHECK(simulator.get_fills()[0].fees == doctest::Approx(expected_fees(kQty, kFeePerShareAlt)));
}

TEST_CASE("reset clears fills and discards pending orders") {
    OrderBook book = make_two_sided_book();
    ExecutionSimulator simulator(kFeePerShare, kLatencyNs, kSlippageBps);
    simulator.set_order_book(&book);

    simulator.submit_order(Order{1ull, OrderSide::BUY, OrderType::MARKET, 0.0, 5ull, 1000ull});
    simulator.process_orders(1000ull + kLatencyNs);
    REQUIRE(simulator.get_fills().size() == 1u);

    simulator.reset();
    CHECK(simulator.get_fills().empty());

    simulator.process_orders(5000000ull);
    CHECK(simulator.get_fills().empty());
}
