#pragma once
#include <vector>
#include <cmath>
#include <algorithm>

class PerformanceMetrics {
public:
    PerformanceMetrics();

    void add_pnl(double pnl);
    void reset();

    double get_total_pnl() const;
    double get_sharpe_ratio(double risk_free_rate = 0.0) const;
    double get_max_drawdown() const;

    const std::vector<double>& get_pnl_history() const { return pnl_history_; }

private:
    std::vector<double> pnl_history_;
    std::vector<double> equity_curve_;
    double total_pnl_;
    double peak_equity_;
};
