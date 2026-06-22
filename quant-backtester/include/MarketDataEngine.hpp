#pragma once
#include "MarketDataEvent.hpp"
#include <vector>
#include <functional>

using MarketDataCallback = std::function<void(const MarketDataEvent&)>;

class MarketDataEngine {
public:
    MarketDataEngine();

    void subscribe(MarketDataCallback callback);
    void add_event(const MarketDataEvent& event);
    void process_all_events();
    void clear_events();

    const std::vector<MarketDataEvent>& get_events() const { return events_; }

private:
    std::vector<MarketDataEvent> events_;
    std::vector<MarketDataCallback> callbacks_;
};
