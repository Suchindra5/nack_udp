#include "loss_emulator.h"
#include <chrono>

LossEmulator::LossEmulator(double drop_rate) 
    : drop_probability_(drop_rate), 
      rng_(std::chrono::steady_clock::now().time_since_epoch().count()), 
      dist_(0.0, 1.0) {}

bool LossEmulator::should_drop() {
    if (drop_probability_ <= 0.0) return false;
    if (drop_probability_ >= 1.0) return true;

    // Generate random number between 0.0 and 1.0; if it's less than drop rate, drop it!
    return dist_(rng_) < drop_probability_;
}

void LossEmulator::set_drop_rate(double drop_rate) {
    drop_probability_ = drop_rate;
}