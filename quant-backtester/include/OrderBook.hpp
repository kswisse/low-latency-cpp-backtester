#pragma once
#include <array>
#include <vector>
#include <cstdint>
#include <algorithm>

struct PriceLevel {
    double price;
    uint64_t quantity;

    PriceLevel() : price(0.0), quantity(0) {}
    PriceLevel(double p, uint64_t q) : price(p), quantity(q) {}
};

constexpr size_t MAX_PRICE_LEVELS = 100;

class OrderBook {
public:
    OrderBook();

    void update_bid(double price, uint64_t quantity);
    void update_ask(double price, uint64_t quantity);

    double get_best_bid() const;
    double get_best_ask() const;
    double get_mid_price() const;

    const std::array<PriceLevel, MAX_PRICE_LEVELS>& get_bids() const { return bids_; }
    const std::array<PriceLevel, MAX_PRICE_LEVELS>& get_asks() const { return asks_; }

    size_t get_bid_count() const { return bid_count_; }
    size_t get_ask_count() const { return ask_count_; }

private:
    std::array<PriceLevel, MAX_PRICE_LEVELS> bids_;
    std::array<PriceLevel, MAX_PRICE_LEVELS> asks_;
    size_t bid_count_;
    size_t ask_count_;

    void sort_bids();
    void sort_asks();
};
