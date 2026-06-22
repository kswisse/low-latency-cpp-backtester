#pragma once
#include <cstdint>

enum class EventType {
    BID_UPDATE,
    ASK_UPDATE,
    TRADE
};

struct MarketDataEvent {
    EventType type;
    double price;
    uint64_t quantity;
    uint64_t timestamp;
};
