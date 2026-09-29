#include <doctest/doctest.h>

#include "PerformanceMetrics.hpp"

#include <cmath>
#include <vector>

TEST_CASE("fresh metrics report zeros") {
    PerformanceMetrics metrics;
    CHECK(metrics.get_total_pnl() == 0.0);
    CHECK(metrics.get_sharpe_ratio() == 0.0);
    CHECK(metrics.get_sharpe_ratio(0.05) == 0.0);
    CHECK(metrics.get_max_drawdown() == 0.0);
    CHECK(metrics.get_pnl_history().empty());
}

TEST_CASE("total pnl accumulates every addition") {
    PerformanceMetrics metrics;
    metrics.add_pnl(1.5);
    metrics.add_pnl(-2.0);
    metrics.add_pnl(3.0);

    CHECK(metrics.get_total_pnl() == doctest::Approx(2.5));
    REQUIRE(metrics.get_pnl_history().size() == 3u);
    CHECK(metrics.get_pnl_history()[0] == doctest::Approx(1.5));
    CHECK(metrics.get_pnl_history()[1] == doctest::Approx(-2.0));
    CHECK(metrics.get_pnl_history()[2] == doctest::Approx(3.0));
}

TEST_CASE("sharpe ratio uses the sample standard deviation of pnl returns") {
    PerformanceMetrics metrics;
    metrics.add_pnl(1.0);
    metrics.add_pnl(2.0);
    metrics.add_pnl(3.0);
    metrics.add_pnl(4.0);

    const double mean = 2.5;
    const double sample_stddev = std::sqrt(5.0 / 3.0);
    CHECK(metrics.get_sharpe_ratio() == doctest::Approx(mean / sample_stddev));

    const double risk_free = 0.5;
    CHECK(metrics.get_sharpe_ratio(risk_free) == doctest::Approx((mean - risk_free) / sample_stddev));
}

TEST_CASE("sharpe ratio is zero when there is no dispersion") {
    PerformanceMetrics metrics;
    metrics.add_pnl(5.0);
    metrics.add_pnl(5.0);
    metrics.add_pnl(5.0);
    CHECK(metrics.get_sharpe_ratio() == 0.0);
}

TEST_CASE("sharpe ratio needs at least two observations") {
    PerformanceMetrics metrics;
    CHECK(metrics.get_sharpe_ratio() == 0.0);
    metrics.add_pnl(10.0);
    CHECK(metrics.get_sharpe_ratio() == 0.0);
}

TEST_CASE("max drawdown measures the deepest peak-to-trough decline of the equity curve") {
    PerformanceMetrics metrics;
    metrics.add_pnl(100.0);   // equity 100
    metrics.add_pnl(-50.0);   // equity 50
    metrics.add_pnl(-25.0);   // equity 25 -> 75% below peak
    CHECK(metrics.get_max_drawdown() == doctest::Approx(0.75));
}

TEST_CASE("max drawdown is zero for a monotonically rising equity curve") {
    PerformanceMetrics metrics;
    metrics.add_pnl(10.0);
    metrics.add_pnl(20.0);
    metrics.add_pnl(30.0);
    CHECK(metrics.get_max_drawdown() == doctest::Approx(0.0));
}

TEST_CASE("max drawdown ignores drawdowns that are not the deepest one") {
    PerformanceMetrics metrics;
    metrics.add_pnl(10.0);    // equity 10 (peak)
    metrics.add_pnl(-4.0);    // equity 6  -> dd 0.40
    metrics.add_pnl(3.0);     // equity 9  -> recovered
    metrics.add_pnl(-1.0);    // equity 8  -> dd 0.10 from peak 10
    CHECK(metrics.get_max_drawdown() == doctest::Approx(0.40));
}

TEST_CASE("reset clears history, totals and derived statistics") {
    PerformanceMetrics metrics;
    metrics.add_pnl(100.0);
    metrics.add_pnl(-80.0);
    REQUIRE(metrics.get_total_pnl() == doctest::Approx(20.0));
    REQUIRE(metrics.get_max_drawdown() > 0.0);

    metrics.reset();

    CHECK(metrics.get_total_pnl() == 0.0);
    CHECK(metrics.get_max_drawdown() == 0.0);
    CHECK(metrics.get_sharpe_ratio() == 0.0);
    CHECK(metrics.get_pnl_history().empty());

    metrics.add_pnl(7.0);
    CHECK(metrics.get_total_pnl() == doctest::Approx(7.0));
    CHECK(metrics.get_pnl_history().size() == 1u);
}
