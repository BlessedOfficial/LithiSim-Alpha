#include "simulate.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <thread>
#include <vector>

namespace {

class ConsoleSimulationLogger final : public SimulationLogger {
   public:
    void log_state(int, const Truck& truck) override { truck.log_state(); }

    void log_wear(int tick, int truck_index, int wear, int health) override {
        std::cout << "[tick " << tick << "] truck " << (truck_index + 1)
                  << " wear=" << wear
                  << " health=" << health << '\n';
    }

    void log_breakdown(int tick, int truck_index, const char* reason) override {
        if (std::string(reason) == "forced_breakdown") {
            std::cout << "[tick " << tick << "] truck " << (truck_index + 1)
                      << " forced breakdown (health<15)\n";
            return;
        }
        std::cout << "[tick " << tick << "] truck " << (truck_index + 1)
                  << " breakdown while idle (health<30)\n";
    }

    void log_dispatch(int tick, int events, int calls, int successful) override {
        std::cout << "[tick " << tick << "] dispatch triggered (events=" << events
                  << ", calls=" << calls
                  << ", successful=" << successful << ")\n";
    }

    void log_dispatch_skip(int tick) override {
        std::cout << "[tick " << tick << "] dispatch skipped (events=0)\n";
    }
};

void initialize_wear_schedule(std::vector<int>& next_wear_tick, MathModels& math_models, const SimulationConfig& config) {
    for (int i = 0; i < static_cast<int>(next_wear_tick.size()); ++i) {
        next_wear_tick[i] = math_models.sample_uniform_int(config.wear_interval_min, config.wear_interval_max);
    }
}

void apply_wear_if_due(Truck& truck, int truck_index, int tick, int& next_wear_tick, MathModels& math_models, const SimulationConfig& config, SimulationLogger& logger) {
    if (tick < next_wear_tick) {
        return;
    }

    const int wear = math_models.sample_uniform_int(config.wear_amount_min, config.wear_amount_max);
    truck.set_health(truck.get_health() - wear);
    next_wear_tick = tick + math_models.sample_uniform_int(config.wear_interval_min, config.wear_interval_max);
    logger.log_wear(tick, truck_index, wear, truck.get_health());
}

void apply_health_breakdown_rules(Truck& truck, int truck_index, int tick, SimulationLogger& logger) {
    if (truck.get_health() < 15 && truck.get_state() != BROKEN) {
        logger.log_breakdown(tick, truck_index, "forced_breakdown");
        truck.break_down();
        return;
    }

    if (truck.get_health() < 30 && truck.get_state() == IDLE) {
        logger.log_breakdown(tick, truck_index, "idle_breakdown");
        truck.break_down();
    }
}

bool repair_if_broken(Truck& truck) {
    if (truck.get_state() != BROKEN) {
        return false;
    }
    truck.repair();
    truck.log_state();
    return true;
}

void update_speed_for_tick(Truck& truck, MathModels& math_models, const SimulationConfig& config) {
    const double tick_speed = static_cast<double>(math_models.sample_uniform_int(config.tick_speed_min, config.tick_speed_max));
    truck.set_speed(tick_speed);
}

void advance_dispatched_truck(Truck& truck) {
    if (!truck.is_dispatched()) {
        return;
    }
    truck.move();
    truck.unload();
}

void run_dispatch_if_due(Truck trucks[], int truck_count, int tick, int& next_dispatch_tick, MathModels& math_models, const SimulationConfig& config, SimulationLogger& logger) {
    if (tick != next_dispatch_tick) {
        return;
    }

    const int dispatch_events = math_models.sample_poisson(config.dispatch_lambda);
    if (dispatch_events > 0) {
        int successful_dispatches = 0;
        for (int i = 0; i < dispatch_events; ++i) {
            if (dispatch(trucks, truck_count, math_models)) {
                ++successful_dispatches;
            }
        }
        logger.log_dispatch(tick, dispatch_events, dispatch_events, successful_dispatches);
    } else {
        logger.log_dispatch_skip(tick);
    }

    next_dispatch_tick += math_models.sample_uniform_int(config.dispatch_interval_min, config.dispatch_interval_max);
}

}  // namespace

bool dispatch(Truck trucks[], int truck_count, MathModels& math_models) {
    if (truck_count <= 0) {
        return false;
    }

    int selected_index = -1;
    int best_health = -1;
    for (int i = 0; i < truck_count; ++i) {
        if (trucks[i].get_state() != IDLE || trucks[i].is_dispatched()) {
            continue;
        }
        if (trucks[i].get_health() > best_health) {
            best_health = trucks[i].get_health();
            selected_index = i;
        }
    }

    if (selected_index < 0) {
        return false;
    }

    // speed is mean travel time, so rate = 1 / mean for exponential distribution.
    const double rate = 1.0 / trucks[selected_index].get_speed();
    const double sampled_time = math_models.sample_exponential(rate);
    const int travel_seconds = std::max(1, static_cast<int>(std::ceil(sampled_time)));

    // Only one truck is selected and dispatched per call.
    trucks[selected_index].set_dispatched(true);
    trucks[selected_index].load(travel_seconds);
    return true;
}

void simulate(Truck trucks[], int truck_count, int total_timesteps, MathModels& math_models) {
    const SimulationConfig config;
    simulate(trucks, truck_count, total_timesteps, math_models, config);
}

void simulate(Truck trucks[], int truck_count, int total_timesteps, MathModels& math_models, const SimulationConfig& config) {
    ConsoleSimulationLogger logger;
    simulate(trucks, truck_count, total_timesteps, math_models, config, logger);
}

void simulate(Truck trucks[], int truck_count, int total_timesteps, MathModels& math_models, const SimulationConfig& config, SimulationLogger& logger) {
    if (truck_count <= 0) {
        return;
    }

    int next_dispatch_tick = math_models.sample_uniform_int(config.dispatch_interval_min, config.dispatch_interval_max);
    std::vector<int> next_wear_tick(truck_count);
    initialize_wear_schedule(next_wear_tick, math_models, config);

    for (int tick = 1; total_timesteps <= 0 || tick <= total_timesteps; ++tick) {
        for (int i = 0; i < truck_count; ++i) {
            apply_wear_if_due(trucks[i], i, tick, next_wear_tick[i], math_models, config, logger);
            apply_health_breakdown_rules(trucks[i], i, tick, logger);

            if (repair_if_broken(trucks[i])) {
                logger.log_state(tick, trucks[i]);
                continue;
            }

            update_speed_for_tick(trucks[i], math_models, config);
            logger.log_state(tick, trucks[i]);
            advance_dispatched_truck(trucks[i]);
        }

        run_dispatch_if_due(trucks, truck_count, tick, next_dispatch_tick, math_models, config, logger);
        std::this_thread::sleep_for(std::chrono::milliseconds(config.tick_delay_ms));
    }
}

void repair_trucks(Truck trucks[], int truck_count) {
    for (int i = 0; i < truck_count; ++i) {
        if (trucks[i].get_state() == BROKEN) {
            trucks[i].repair();
        }
    }
}