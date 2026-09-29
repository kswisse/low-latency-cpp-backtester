include(FetchContent)

# doctest v2.4.11 declares cmake_minimum_required < 3.5, which CMake >= 4.0
# rejects; harmless no-op on older CMake (CI).
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

FetchContent_Declare(doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG        v2.4.11
    GIT_SHALLOW    TRUE
)

# Keep doctest's install() rules out of this project's `cmake --install`.
set(DOCTEST_NO_INSTALL ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(doctest)

# Compile doctest's implementation (with main) once; every test executable
# links the same objects instead of recompiling the implementation per file.
add_library(doctest_main OBJECT doctest_main.cpp)
target_link_libraries(doctest_main PRIVATE doctest::doctest)

set(TEST_TARGETS
    test_order_book
    test_performance_metrics
    test_execution_simulator
    test_market_data_engine
    test_backtester_contract
    test_simple_market_maker
)

foreach(test_target IN LISTS TEST_TARGETS)
    add_executable(${test_target} ${test_target}.cpp $<TARGET_OBJECTS:doctest_main>)
    target_link_libraries(${test_target} PRIVATE quant_backtester_lib doctest::doctest)
    add_test(NAME ${test_target} COMMAND ${test_target})
endforeach()
