#pragma once

#include "math_models.h"
#include "simulation_logger.h"
#include "truck.h"
#include <vector>

enum class LoggingMode {
    CONSOLE,
    CSV
};

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
    int pause_poll_delay_ms = 100;
    int purchase_interval_min = 0;
    int purchase_interval_max = 0;
    int max_truck_count = 0;  // 0 means unlimited
    LoggingMode logging_mode = LoggingMode::CONSOLE;
    bool enable_pause_resume_controls = true;
};

bool dispatch(std::vector<Truck>& trucks);
void simulate(std::vector<Truck>& trucks, int total_timesteps, MathModels& math_models);
void simulate(std::vector<Truck>& trucks, int total_timesteps, MathModels& math_models, const SimulationConfig& config);
void simulate(std::vector<Truck>& trucks, int total_timesteps, MathModels& math_models, const SimulationConfig& config, SimulationLogger& logger);
void repair_trucks(std::vector<Truck>& trucks);
