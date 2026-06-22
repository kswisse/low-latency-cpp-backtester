#include "MarketDataEngine.hpp"

MarketDataEngine::MarketDataEngine() {
    events_.reserve(1000000);
    callbacks_.reserve(10);
}

void MarketDataEngine::subscribe(MarketDataCallback callback) {
    callbacks_.push_back(std::move(callback));
}

void MarketDataEngine::add_event(const MarketDataEvent& event) {
    events_.push_back(event);
}

void MarketDataEngine::process_all_events() {
    for (const auto& event : events_) {
        for (const auto& callback : callbacks_) {
            callback(event);
        }
    }
}

void MarketDataEngine::clear_events() {
    events_.clear();
}
