#include <iostream>
#include "math_models.h"
#include "simulate.h"
#include "truck.h"

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    Truck trucks[] = {Truck("ABCD")};
    MathModels math_models(42);
    SimulationConfig config;
    simulate(trucks, 1, -1, math_models, config);

    return 0;
}
