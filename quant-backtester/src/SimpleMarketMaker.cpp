#include "SimpleMarketMaker.hpp"

SimpleMarketMaker::SimpleMarketMaker(double spread_bps, uint64_t order_size)
    : spread_bps_(spread_bps), order_size_(order_size), last_event_count_(0),
      position_(0.0), next_order_id_(1) {}

void SimpleMarketMaker::on_market_data(const MarketDataEvent& event, const OrderBook& order_book) {
    double mid_price = order_book.get_mid_price();
    if (mid_price == 0.0) return;

    double spread = (spread_bps_ / 10000.0) * mid_price;
    double bid_price = mid_price - spread / 2.0;
    double ask_price = mid_price + spread / 2.0;

    Order buy_order{
        next_order_id_++,
        OrderSide::BUY,
        OrderType::LIMIT,
        bid_price,
        order_size_,
        event.timestamp
    };
    submit_order(buy_order);

    Order sell_order{
        next_order_id_++,
        OrderSide::SELL,
        OrderType::LIMIT,
        ask_price,
        order_size_,
        event.timestamp
    };
    submit_order(sell_order);
}

void SimpleMarketMaker::on_fill(const Fill& fill) {
}
