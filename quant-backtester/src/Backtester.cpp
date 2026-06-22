#include "Backtester.hpp"

Backtester::Backtester() : current_equity_(1000000.0) {
    simulator_.set_order_book(&order_book_);
}

void Backtester::set_strategy(std::unique_ptr<StrategyInterface> strategy) {
    strategy_ = std::move(strategy);

    if (strategy_) {
        strategy_->set_order_submit_callback([this](const Order& order) {
            simulator_.submit_order(order);
        });
    }
}

void Backtester::add_market_event(const MarketDataEvent& event) {
    data_engine_.add_event(event);
}

void Backtester::run() {
    data_engine_.subscribe([this](const MarketDataEvent& event) {
        if (event.type == EventType::BID_UPDATE) {
            order_book_.update_bid(event.price, event.quantity);
        } else if (event.type == EventType::ASK_UPDATE) {
            order_book_.update_ask(event.price, event.quantity);
        }

        if (strategy_) {
            strategy_->on_market_data(event, order_book_);
        }

        simulator_.process_orders(event.timestamp);

        for (const auto& fill : simulator_.get_fills()) {
            double pnl = 0.0;
            if (strategy_) {
                strategy_->on_fill(fill);
            }
        }
    });

    data_engine_.process_all_events();
}

void Backtester::reset() {
    data_engine_.clear_events();
    simulator_.reset();
    metrics_.reset();
    current_equity_ = 1000000.0;
}
