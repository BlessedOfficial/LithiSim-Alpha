#pragma once

#include <random>

class MathModels {
private:
    std::mt19937 rng;

public:
    explicit MathModels(unsigned int seed = std::random_device{}());

    // Counts number of events in a fixed interval.
    int sample_poisson(double lambda);

    // Returns waiting time between events.
    double sample_exponential(double rate);

    // Returns a random integer in [min_value, max_value].
    int sample_uniform_int(int min_value, int max_value);
};
