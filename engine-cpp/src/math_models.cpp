#include "math_models.h"

#include <stdexcept>

MathModels::MathModels(unsigned int seed) : rng(seed) {}

int MathModels::sample_poisson(double lambda) {
    if(lambda <= 0.0) {
        throw std::invalid_argument("lambda must be > 0 for Poisson");
    }

    std::poisson_distribution<int> distribution(lambda);
    return distribution(rng);
}

double MathModels::sample_exponential(double rate) {
    if (rate <= 0.0) {
        throw std::invalid_argument("rate must be > 0 for Exponential");
    }
    std::exponential_distribution<double> distribution(rate);
    return distribution(rng);

}

int MathModels::sample_uniform_int(int min_value, int max_value) {
    if (min_value > max_value) {
        throw std::invalid_argument("min_value must be <= max_value");
    }
    std::uniform_int_distribution<int> distribution(min_value, max_value);
    return distribution(rng);
}


