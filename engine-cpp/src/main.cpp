#include <iostream>
#include "math_models.h"
#include "simulate.h"
#include "truck.h"
#include <vector>

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    std::vector<Truck> trucks = {
        Truck("ABCD"),
       
    };
    MathModels math_models(42);
    SimulationConfig config;
    config.logging_mode = LoggingMode::CONSOLE;
    config.purchase_interval_min = 300;
    config.purchase_interval_max = 600;
    config.max_truck_count = 12;
    simulate(trucks, -1, math_models, config);

    return 0;
}
