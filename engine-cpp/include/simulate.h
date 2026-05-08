#pragma once

#include "math_models.h"
#include "simulation_logger.h"
#include "truck.h"

struct SimulationConfig {
    double dispatch_lambda = 0.55;
    int dispatch_interval_min = 20;
    int dispatch_interval_max = 25;
    int wear_interval_min = 5;
    int wear_interval_max = 20;
    int wear_amount_min = 1;
    int wear_amount_max = 3;
    int tick_speed_min = 40;
    int tick_speed_max = 100;
    int tick_delay_ms = 500;
};

bool dispatch(Truck trucks[], int truck_count, MathModels& math_models);
void simulate(Truck trucks[], int truck_count, int total_timesteps, MathModels& math_models);
void simulate(Truck trucks[], int truck_count, int total_timesteps, MathModels& math_models, const SimulationConfig& config);
void simulate(Truck trucks[], int truck_count, int total_timesteps, MathModels& math_models, const SimulationConfig& config, SimulationLogger& logger);
void repair_trucks(Truck trucks[], int truck_count);
