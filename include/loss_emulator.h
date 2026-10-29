#ifndef LOSS_EMULATOR_H
#define LOSS_EMULATOR_H

#include <cstdint>
#include <vector>
#include <random>

class LossEmulator {
private:
    double drop_probability_; // e.g., 0.1 for 10% packet loss
    std::mt19937 rng_;
    std::uniform_real_distribution<double> dist_;

public:
    // drop_rate should be between 0.0 (0%) and 1.0 (100%)
    LossEmulator(double drop_rate = 0.1);

    // Determines whether a packet should be dropped or allowed through
    bool should_drop();

    void set_drop_rate(double drop_rate);
};

#endif // LOSS_EMULATOR_H