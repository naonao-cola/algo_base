#pragma once

#include <chrono>
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;

class TimeCost
{
public:
    TimeCost(bool bStart=false) {
        is_stop_ = false;
        if (bStart) {
            start();
        }
    }

    void start() {
        start_time_ = high_resolution_clock::now();
        last_time_ = start_time_;
    }

    void stop() {
        high_resolution_clock::time_point now = high_resolution_clock::now();
        time_interval_ = std::chrono::duration_cast<milliseconds>(now - start_time_);
        is_stop_ = true;
    }

    long long  get_cost_time(bool from_last = true) {
        high_resolution_clock::time_point now = high_resolution_clock::now();
        if (from_last) {
            time_interval_ = std::chrono::duration_cast<milliseconds>(now - last_time_);
        } else {
            time_interval_ = std::chrono::duration_cast<milliseconds>(now - start_time_);
        }
        last_time_ = now;
        return time_interval_.count();
    }

private:
    bool is_stop_{false};
    high_resolution_clock::time_point start_time_;
    high_resolution_clock::time_point end_time_;
    high_resolution_clock::time_point last_time_;
    milliseconds time_interval_;
};
