# Low-Latency C++ Backtester

A high-performance, low-latency **L2 order book and backtesting engine** written in C++20 for quant trading strategies.

> Research and paper-trading tooling only — no live trading, no financial advice.

## Features

- **L2 Order Book** — price-level book with incremental updates
- **Market Data Engine** — replays event streams (bid/ask updates) through the book
- **Execution Simulator** — simulates order fills against the live book
- **Performance Metrics** — latency and throughput measurement
- **Strategy Interface** — plug in your own strategy; includes a sample `SimpleMarketMaker`
- **Benchmark demo** — `src/main.cpp` generates 1M mock events and reports events/sec

## Structure

```
quant-backtester/
├── CMakeLists.txt
├── include/          # Public headers
│   ├── OrderBook.hpp
│   ├── MarketDataEngine.hpp
│   ├── ExecutionSimulator.hpp
│   ├── PerformanceMetrics.hpp
│   ├── Backtester.hpp
│   ├── StrategyInterface.hpp
│   └── SimpleMarketMaker.hpp
└── src/              # Implementation + benchmark entry point
    └── main.cpp
```

## Build

Requirements: **CMake 3.16+** and a **C++20** compiler (GCC 12+, Clang 15+, or MSVC 2022+).

```bash
cmake -S quant-backtester -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

Run the benchmark:

```bash
./build/quant_backtester      # Windows: build\Release\quant_backtester.exe with MSVC
```

Example output:

```
Generating 1000000 mock events...
Running backtest...

=== PERFORMANCE METRICS ===
Time taken: ... ms
Events per second: ...
=== TRADING METRICS ===
Total fills: ...
```

## CI

GitHub Actions configures and builds the project with GCC on every push and pull request to `main`.

## License

MIT — see [LICENSE](LICENSE).
