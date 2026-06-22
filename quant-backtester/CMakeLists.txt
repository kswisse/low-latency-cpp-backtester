cmake_minimum_required(VERSION 3.16)
project(QuantBacktester VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/O2 /W4 /WX- /GL)
    add_link_options(/LTCG)
else()
    add_compile_options(-O3 -Wall -Wextra -march=native)
endif()

set(SOURCES
    src/OrderBook.cpp
    src/MarketDataEngine.cpp
    src/ExecutionSimulator.cpp
    src/PerformanceMetrics.cpp
    src/Backtester.cpp
    src/SimpleMarketMaker.cpp
)

add_library(quant_backtester_lib STATIC ${SOURCES})
target_include_directories(quant_backtester_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(quant_backtester src/main.cpp)
target_link_libraries(quant_backtester PRIVATE quant_backtester_lib)
