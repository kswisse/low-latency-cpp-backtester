# Low-Latency C++ Backtester

[![CI](https://github.com/kswisse/low-latency-cpp-backtester/actions/workflows/ci.yml/badge.svg)](https://github.com/kswisse/low-latency-cpp-backtester/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

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
├── include/                  # Public headers
│   ├── Backtester.hpp
│   ├── ExecutionSimulator.hpp
│   ├── MarketDataEngine.hpp
│   ├── MarketDataEvent.hpp
│   ├── Order.hpp
│   ├── OrderBook.hpp
│   ├── PerformanceMetrics.hpp
│   ├── SimpleMarketMaker.hpp
│   └── StrategyInterface.hpp
├── src/                      # Implementation + benchmark entry point
│   ├── Backtester.cpp
│   ├── ExecutionSimulator.cpp
│   ├── MarketDataEngine.cpp
│   ├── OrderBook.cpp
│   ├── PerformanceMetrics.cpp
│   ├── SimpleMarketMaker.cpp
│   └── main.cpp
└── tests/                    # doctest suite, wired into CTest
    ├── CMakeLists.txt
    ├── doctest_main.cpp
    ├── test_support.hpp
    ├── test_backtester_contract.cpp
    ├── test_execution_simulator.cpp
    ├── test_market_data_engine.cpp
    ├── test_order_book.cpp
    ├── test_performance_metrics.cpp
    └── test_simple_market_maker.cpp
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

Example output — measured on AMD Ryzen 7 250 (8 cores / 16 threads), 32 GB RAM,
Windows 11, g++ 16.1.0 Release build. Four consecutive runs took 95–107 ms
(103 / 95 / 107 / 99 ms), i.e. ~10M events/sec; every run reported 0 fills:

```
Generating 1000000 mock events...
Running backtest...

=== PERFORMANCE METRICS ===
Time taken: 103 ms
Events per second: 9.70874e+06

=== TRADING METRICS ===
Total fills: 0
```

## Testing

The suite uses [doctest](https://github.com/doctest/doctest) v2.4.11 (fetched by
CMake at configure time): **51 test cases / 248 assertions** across 6 executables
registered with CTest — `test_order_book` (10), `test_performance_metrics` (9),
`test_execution_simulator` (10), `test_market_data_engine` (6),
`test_backtester_contract` (9), `test_simple_market_maker` (7).

The gate CI runs:

```bash
ctest --test-dir build --output-on-failure --no-tests=error
```

## CI

GitHub Actions configures, builds, runs the test suite and a benchmark smoke
test with GCC on every push and pull request to `main`.

## License

MIT — see [LICENSE](LICENSE).
