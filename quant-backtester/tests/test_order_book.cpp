#include <doctest/doctest.h>

#include "OrderBook.hpp"

TEST_CASE("empty book exposes zero prices, zero mid and zero counts") {
    OrderBook book;
    CHECK(book.get_bid_count() == 0u);
    CHECK(book.get_ask_count() == 0u);
    CHECK(book.get_best_bid() == 0.0);
    CHECK(book.get_best_ask() == 0.0);
    CHECK(book.get_mid_price() == 0.0);
}

TEST_CASE("mid price is only available when both sides are quoted") {
    OrderBook book;
    book.update_bid(100.0, 5);

    CHECK(book.get_best_bid() == doctest::Approx(100.0));
    CHECK(book.get_best_ask() == 0.0);
    CHECK(book.get_mid_price() == 0.0);

    book.update_ask(100.4, 7);
    CHECK(book.get_best_ask() == doctest::Approx(100.4));
    CHECK(book.get_mid_price() == doctest::Approx(100.2));
}

TEST_CASE("bids are kept sorted with the highest price first") {
    OrderBook book;
    book.update_bid(99.5, 10);
    book.update_bid(100.5, 20);
    book.update_bid(100.0, 30);

    REQUIRE(book.get_bid_count() == 3u);
    CHECK(book.get_best_bid() == doctest::Approx(100.5));

    const auto& bids = book.get_bids();
    CHECK(bids[0].price == doctest::Approx(100.5));
    CHECK(bids[0].quantity == 20ull);
    CHECK(bids[1].price == doctest::Approx(100.0));
    CHECK(bids[1].quantity == 30ull);
    CHECK(bids[2].price == doctest::Approx(99.5));
    CHECK(bids[2].quantity == 10ull);

    CHECK(bids[0].price > bids[1].price);
    CHECK(bids[1].price > bids[2].price);
}

TEST_CASE("asks are kept sorted with the lowest price first") {
    OrderBook book;
    book.update_ask(102.0, 40);
    book.update_ask(100.5, 50);
    book.update_ask(101.0, 60);

    REQUIRE(book.get_ask_count() == 3u);
    CHECK(book.get_best_ask() == doctest::Approx(100.5));

    const auto& asks = book.get_asks();
    CHECK(asks[0].price == doctest::Approx(100.5));
    CHECK(asks[0].quantity == 50ull);
    CHECK(asks[1].price == doctest::Approx(101.0));
    CHECK(asks[1].quantity == 60ull);
    CHECK(asks[2].price == doctest::Approx(102.0));
    CHECK(asks[2].quantity == 40ull);

    CHECK(asks[0].price < asks[1].price);
    CHECK(asks[1].price < asks[2].price);
}

TEST_CASE("updating an existing price replaces the quantity without adding a level") {
    OrderBook book;
    book.update_bid(100.0, 10);
    book.update_bid(100.0, 25);
    CHECK(book.get_bid_count() == 1u);
    CHECK(book.get_bids()[0].quantity == 25ull);

    book.update_ask(101.0, 4);
    book.update_ask(101.0, 9);
    CHECK(book.get_ask_count() == 1u);
    CHECK(book.get_asks()[0].quantity == 9ull);
}

TEST_CASE("zero quantity removes the matching level") {
    OrderBook book;
    book.update_bid(100.5, 20);
    book.update_bid(100.0, 30);
    book.update_bid(99.5, 40);
    REQUIRE(book.get_bid_count() == 3u);

    book.update_bid(100.0, 0);
    REQUIRE(book.get_bid_count() == 2u);
    CHECK(book.get_bids()[0].price == doctest::Approx(100.5));
    CHECK(book.get_bids()[1].price == doctest::Approx(99.5));
    CHECK(book.get_best_bid() == doctest::Approx(100.5));

    book.update_ask(101.0, 1);
    book.update_ask(102.0, 2);
    book.update_ask(103.0, 3);
    REQUIRE(book.get_ask_count() == 3u);

    book.update_ask(102.0, 0);
    REQUIRE(book.get_ask_count() == 2u);
    CHECK(book.get_asks()[0].price == doctest::Approx(101.0));
    CHECK(book.get_asks()[1].price == doctest::Approx(103.0));
    CHECK(book.get_best_ask() == doctest::Approx(101.0));
}

TEST_CASE("removing a price that is not on the book is a no-op") {
    OrderBook book;
    book.update_bid(100.0, 10);
    book.update_ask(101.0, 20);

    book.update_bid(999.0, 0);
    book.update_ask(999.0, 0);
    CHECK(book.get_bid_count() == 1u);
    CHECK(book.get_ask_count() == 1u);
    CHECK(book.get_best_bid() == doctest::Approx(100.0));
    CHECK(book.get_best_ask() == doctest::Approx(101.0));
}

TEST_CASE("zero quantity on a new price does not insert a level") {
    OrderBook book;
    book.update_bid(100.0, 0);
    book.update_ask(101.0, 0);
    CHECK(book.get_bid_count() == 0u);
    CHECK(book.get_ask_count() == 0u);
    CHECK(book.get_best_bid() == 0.0);
    CHECK(book.get_best_ask() == 0.0);
}

TEST_CASE("bid side is capped at MAX_PRICE_LEVELS entries") {
    OrderBook book;
    double last_inserted_price = 0.0;
    for (size_t i = 0; i < MAX_PRICE_LEVELS; ++i) {
        last_inserted_price = 100.0 + static_cast<double>(i) * 0.01;
        book.update_bid(last_inserted_price, i + 1);
    }
    REQUIRE(book.get_bid_count() == MAX_PRICE_LEVELS);

    CHECK(book.get_best_bid() == doctest::Approx(100.0 + static_cast<double>(MAX_PRICE_LEVELS - 1) * 0.01));
    CHECK(book.get_bids()[0].quantity == MAX_PRICE_LEVELS);

    book.update_bid(150.0, 1);
    CHECK(book.get_bid_count() == MAX_PRICE_LEVELS);
    CHECK(book.get_best_bid() == doctest::Approx(100.0 + static_cast<double>(MAX_PRICE_LEVELS - 1) * 0.01));

    // last_inserted_price (not the literal 100.99) so the update targets the
    // exact same double the loop inserted, bit for bit.
    book.update_bid(last_inserted_price, 55);
    CHECK(book.get_bid_count() == MAX_PRICE_LEVELS);
    CHECK(book.get_bids()[0].quantity == 55ull);
}

TEST_CASE("ask side is capped at MAX_PRICE_LEVELS entries") {
    OrderBook book;
    for (size_t i = 0; i < MAX_PRICE_LEVELS; ++i) {
        book.update_ask(100.0 + static_cast<double>(i) * 0.01, i + 1);
    }
    REQUIRE(book.get_ask_count() == MAX_PRICE_LEVELS);

    CHECK(book.get_best_ask() == doctest::Approx(100.0));
    CHECK(book.get_asks()[0].quantity == 1ull);

    book.update_ask(50.0, 5);
    CHECK(book.get_ask_count() == MAX_PRICE_LEVELS);
    CHECK(book.get_best_ask() == doctest::Approx(100.0));
}
