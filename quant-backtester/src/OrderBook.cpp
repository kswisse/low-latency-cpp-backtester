#include "OrderBook.hpp"

OrderBook::OrderBook() : bid_count_(0), ask_count_(0) {
    bids_.fill(PriceLevel());
    asks_.fill(PriceLevel());
}

void OrderBook::update_bid(double price, uint64_t quantity) {
    for (size_t i = 0; i < bid_count_; ++i) {
        if (bids_[i].price == price) {
            if (quantity == 0) {
                for (size_t j = i; j < bid_count_ - 1; ++j) {
                    bids_[j] = bids_[j + 1];
                }
                --bid_count_;
            } else {
                bids_[i].quantity = quantity;
            }
            sort_bids();
            return;
        }
    }

    if (bid_count_ < MAX_PRICE_LEVELS && quantity > 0) {
        bids_[bid_count_] = PriceLevel(price, quantity);
        ++bid_count_;
        sort_bids();
    }
}

void OrderBook::update_ask(double price, uint64_t quantity) {
    for (size_t i = 0; i < ask_count_; ++i) {
        if (asks_[i].price == price) {
            if (quantity == 0) {
                for (size_t j = i; j < ask_count_ - 1; ++j) {
                    asks_[j] = asks_[j + 1];
                }
                --ask_count_;
            } else {
                asks_[i].quantity = quantity;
            }
            sort_asks();
            return;
        }
    }

    if (ask_count_ < MAX_PRICE_LEVELS && quantity > 0) {
        asks_[ask_count_] = PriceLevel(price, quantity);
        ++ask_count_;
        sort_asks();
    }
}

double OrderBook::get_best_bid() const {
    if (bid_count_ == 0) return 0.0;
    return bids_[0].price;
}

double OrderBook::get_best_ask() const {
    if (ask_count_ == 0) return 0.0;
    return asks_[0].price;
}

double OrderBook::get_mid_price() const {
    double bid = get_best_bid();
    double ask = get_best_ask();
    if (bid == 0.0 || ask == 0.0) return 0.0;
    return (bid + ask) / 2.0;
}

void OrderBook::sort_bids() {
    std::sort(bids_.begin(), bids_.begin() + bid_count_,
        [](const PriceLevel& a, const PriceLevel& b) {
            return a.price > b.price;
        });
}

void OrderBook::sort_asks() {
    std::sort(asks_.begin(), asks_.begin() + ask_count_,
        [](const PriceLevel& a, const PriceLevel& b) {
            return a.price < b.price;
        });
}
