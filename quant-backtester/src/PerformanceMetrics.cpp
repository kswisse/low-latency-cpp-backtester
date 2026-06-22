#include "PerformanceMetrics.hpp"

PerformanceMetrics::PerformanceMetrics()
    : total_pnl_(0.0), peak_equity_(0.0) {
    pnl_history_.reserve(10000);
    equity_curve_.reserve(10000);
}

void PerformanceMetrics::add_pnl(double pnl) {
    pnl_history_.push_back(pnl);
    total_pnl_ += pnl;

    double current_equity = 0.0;
    if (!equity_curve_.empty()) {
        current_equity = equity_curve_.back() + pnl;
    } else {
        current_equity = pnl;
    }
    equity_curve_.push_back(current_equity);

    if (current_equity > peak_equity_) {
        peak_equity_ = current_equity;
    }
}

void PerformanceMetrics::reset() {
    pnl_history_.clear();
    equity_curve_.clear();
    total_pnl_ = 0.0;
    peak_equity_ = 0.0;
}

double PerformanceMetrics::get_total_pnl() const {
    return total_pnl_;
}

double PerformanceMetrics::get_sharpe_ratio(double risk_free_rate) const {
    if (pnl_history_.size() < 2) return 0.0;

    double mean = 0.0;
    for (double pnl : pnl_history_) {
        mean += pnl;
    }
    mean /= pnl_history_.size();

    double variance = 0.0;
    for (double pnl : pnl_history_) {
        variance += (pnl - mean) * (pnl - mean);
    }
    variance /= (pnl_history_.size() - 1);

    double stddev = std::sqrt(variance);

    if (stddev == 0.0) return 0.0;
    return (mean - risk_free_rate) / stddev;
}

double PerformanceMetrics::get_max_drawdown() const {
    if (equity_curve_.empty()) return 0.0;

    double max_dd = 0.0;
    double current_peak = equity_curve_[0];

    for (double equity : equity_curve_) {
        if (equity > current_peak) {
            current_peak = equity;
        }
        double dd = (current_peak - equity) / current_peak;
        if (dd > max_dd) {
            max_dd = dd;
        }
    }

    return max_dd;
}
