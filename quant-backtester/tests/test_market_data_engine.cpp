#include <doctest/doctest.h>

#include "MarketDataEngine.hpp"
#include "test_support.hpp"

TEST_CASE("added events are retained with their payload") {
    MarketDataEngine engine;
    engine.add_event(bid(100.0, 10ull, 1000ull));
    engine.add_event(ask(100.5, 20ull, 2000ull));
    engine.add_event(trade(100.3, 5ull, 3000ull));

    const auto& events = engine.get_events();
    REQUIRE(events.size() == 3u);
    CHECK(events[0].type == EventType::BID_UPDATE);
    CHECK(events[0].price == doctest::Approx(100.0));
    CHECK(events[0].quantity == 10ull);
    CHECK(events[0].timestamp == 1000ull);
    CHECK(events[1].type == EventType::ASK_UPDATE);
    CHECK(events[1].timestamp == 2000ull);
    CHECK(events[2].type == EventType::TRADE);
    CHECK(events[2].timestamp == 3000ull);
}

TEST_CASE("processing delivers events to every subscriber in insertion order") {
    MarketDataEngine engine;
    engine.add_event(bid(100.0, 10ull, 1000ull));
    engine.add_event(bid(100.1, 11ull, 2000ull));
    engine.add_event(bid(100.2, 12ull, 3000ull));

    std::vector<MarketDataEvent> first_subscriber;
    std::vector<MarketDataEvent> second_subscriber;
    engine.subscribe([&first_subscriber](const MarketDataEvent& event) { first_subscriber.push_back(event); });
    engine.subscribe([&second_subscriber](const MarketDataEvent& event) { second_subscriber.push_back(event); });

    engine.process_all_events();

    REQUIRE(first_subscriber.size() == 3u);
    REQUIRE(second_subscriber.size() == 3u);
    for (size_t i = 0; i < 3u; ++i) {
        CHECK(first_subscriber[i].timestamp == (i + 1) * 1000u);
        CHECK(second_subscriber[i].timestamp == (i + 1) * 1000u);
    }
}

TEST_CASE("processing does not consume events") {
    MarketDataEngine engine;
    engine.add_event(bid(100.0, 10ull, 1000ull));

    size_t received = 0;
    engine.subscribe([&received](const MarketDataEvent&) { ++received; });

    engine.process_all_events();
    engine.process_all_events();

    CHECK(received == 2u);
    CHECK(engine.get_events().size() == 1u);
}

TEST_CASE("a subscriber added after events were queued still receives them") {
    MarketDataEngine engine;
    engine.add_event(bid(99.0, 1ull, 10ull));
    engine.add_event(bid(99.1, 2ull, 20ull));

    size_t received = 0;
    engine.subscribe([&received](const MarketDataEvent&) { ++received; });
    engine.process_all_events();

    CHECK(received == 2u);
}

TEST_CASE("clear_events drops queued events so nothing is delivered") {
    MarketDataEngine engine;
    engine.add_event(bid(100.0, 10ull, 1000ull));
    engine.add_event(bid(100.1, 11ull, 2000ull));

    size_t received = 0;
    engine.subscribe([&received](const MarketDataEvent&) { ++received; });

    engine.clear_events();
    CHECK(engine.get_events().empty());

    engine.process_all_events();
    CHECK(received == 0u);
}

TEST_CASE("processing an empty engine with no subscribers is safe") {
    MarketDataEngine engine;
    engine.process_all_events();
    CHECK(engine.get_events().empty());
}
