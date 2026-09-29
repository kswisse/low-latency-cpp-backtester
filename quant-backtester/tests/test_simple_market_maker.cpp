#include <doctest/doctest.h>

#include "ExecutionSimulator.hpp"
#include "SimpleMarketMaker.hpp"
#include "test_support.hpp"

namespace {

// SimpleMarketMaker's default arguments (include/SimpleMarketMaker.hpp:7).
// The tests below default-construct the maker and assert against these values,
// so a production default change fails the expectations instead of hiding it.
constexpr double kSpreadBps = 10.0;
constexpr uint64_t kOrderSize = 100;

struct ExpectedQuotes {
    double mid;
    double bid;
    double ask;
};

// Test-side oracle for the quote math (mid; spread = bps/scale * mid;
// quoted bid/ask = mid -/+ spread/2). Recomputed here instead of calling
// SimpleMarketMaker so the assertions are not tautological.
ExpectedQuotes expected_quotes(double best_bid, double best_ask, double spread_bps) {
    const double mid = (best_bid + best_ask) / 2.0;
    const double spread = (spread_bps / kBpsScale) * mid;
    return ExpectedQuotes{mid, mid - spread / 2.0, mid + spread / 2.0};
}

}  // namespace

TEST_CASE("market maker quotes a symmetric two-sided spread around the mid") {
    OrderBook book = make_two_sided_book();

    SimpleMarketMaker market_maker;
    std::vector<Order> submitted;
    record_orders(market_maker, submitted);

    market_maker.on_market_data(bid(100.2, 20ull, 5000ull), book);

    REQUIRE(submitted.size() == 2u);

    const ExpectedQuotes quotes = expected_quotes(100.0, 100.2, kSpreadBps);

    const Order& buy = submitted[0];
    CHECK(buy.side == OrderSide::BUY);
    CHECK(buy.type == OrderType::LIMIT);
    CHECK(buy.quantity == kOrderSize);
    CHECK(buy.timestamp == 5000ull);
    CHECK(buy.order_id == 1ull);
    CHECK(buy.price == doctest::Approx(quotes.bid));

    const Order& sell = submitted[1];
    CHECK(sell.side == OrderSide::SELL);
    CHECK(sell.type == OrderType::LIMIT);
    CHECK(sell.quantity == kOrderSize);
    CHECK(sell.timestamp == 5000ull);
    CHECK(sell.order_id == 2ull);
    CHECK(sell.price == doctest::Approx(quotes.ask));

    CHECK(buy.price < quotes.mid);
    CHECK(sell.price > quotes.mid);
}

TEST_CASE("market maker honours custom spread and order size") {
    OrderBook book;
    book.update_bid(50.0, 10);
    book.update_ask(50.1, 10);

    SimpleMarketMaker market_maker(20.0, 50);
    std::vector<Order> submitted;
    record_orders(market_maker, submitted);

    market_maker.on_market_data(bid(50.1, 10ull, 7ull), book);

    REQUIRE(submitted.size() == 2u);
    const ExpectedQuotes quotes = expected_quotes(50.0, 50.1, 20.0);
    CHECK(submitted[0].price == doctest::Approx(quotes.bid));
    CHECK(submitted[1].price == doctest::Approx(quotes.ask));
    CHECK(submitted[0].quantity == 50ull);
    CHECK(submitted[1].quantity == 50ull);
}

TEST_CASE("market maker stays flat when the book is empty") {
    OrderBook book;

    SimpleMarketMaker market_maker;
    std::vector<Order> submitted;
    record_orders(market_maker, submitted);

    market_maker.on_market_data(bid(100.0, 10ull, 1000ull), book);

    CHECK(submitted.empty());
}

TEST_CASE("market maker stays flat on a one-sided book") {
    OrderBook book;
    book.update_bid(100.0, 10);

    SimpleMarketMaker market_maker;
    std::vector<Order> submitted;
    record_orders(market_maker, submitted);

    market_maker.on_market_data(bid(100.0, 10ull, 1000ull), book);

    CHECK(submitted.empty());
}

TEST_CASE("order ids increment across market data events") {
    OrderBook book = make_two_sided_book();

    SimpleMarketMaker market_maker;
    std::vector<Order> submitted;
    record_orders(market_maker, submitted);

    market_maker.on_market_data(bid(100.2, 20ull, 1000ull), book);
    market_maker.on_market_data(bid(100.4, 30ull, 2000ull), book);

    REQUIRE(submitted.size() == 4u);
    CHECK(submitted[0].order_id == 1ull);
    CHECK(submitted[1].order_id == 2ull);
    CHECK(submitted[2].order_id == 3ull);
    CHECK(submitted[3].order_id == 4ull);
    CHECK(submitted[2].timestamp == 2000ull);
    CHECK(submitted[3].timestamp == 2000ull);
}

TEST_CASE("market maker without a registered callback does not crash") {
    OrderBook book = make_two_sided_book();

    SimpleMarketMaker market_maker;
    market_maker.on_market_data(bid(100.2, 20ull, 1000ull), book);
}

TEST_CASE("market maker quotes execute against the book through the execution simulator") {
    OrderBook book = make_two_sided_book();

    SimpleMarketMaker market_maker;
    ExecutionSimulator simulator;
    simulator.set_order_book(&book);
    market_maker.set_order_submit_callback(
        [&simulator](const Order& order) { simulator.submit_order(order); });

    const uint64_t quote_timestamp = 1000000ull;
    market_maker.on_market_data(bid(100.2, 20ull, quote_timestamp), book);
    CHECK(simulator.get_fills().empty());

    simulator.process_orders(quote_timestamp + kLatencyNs);

    const auto& fills = simulator.get_fills();
    REQUIRE(fills.size() == 2u);

    const ExpectedQuotes quotes = expected_quotes(100.0, 100.2, kSpreadBps);

    CHECK(fills[0].order_id == 1ull);
    CHECK(fills[0].fill_price == doctest::Approx(expected_fill_price(quotes.bid, OrderSide::BUY, kSlippageBps)));
    CHECK(fills[0].fill_quantity == kOrderSize);
    CHECK(fills[0].timestamp == quote_timestamp + kLatencyNs);
    CHECK(fills[0].fees == doctest::Approx(expected_fees(kOrderSize, kFeePerShare)));

    CHECK(fills[1].order_id == 2ull);
    CHECK(fills[1].fill_price == doctest::Approx(expected_fill_price(quotes.ask, OrderSide::SELL, kSlippageBps)));
    CHECK(fills[1].fill_quantity == kOrderSize);
    CHECK(fills[1].fees == doctest::Approx(expected_fees(kOrderSize, kFeePerShare)));
}
