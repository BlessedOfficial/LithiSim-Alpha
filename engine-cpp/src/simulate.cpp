#include "simulate.h"
#include "truck.h"
#include "csv_simulation_logger.h"

#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#ifdef _WIN32
#include <conio.h>
#endif

namespace {

class ConsoleSimulationLogger final : public SimulationLogger {
   public:
    void log_state(int, const Truck& truck) override {
        std::cout << "truck " << truck.get_id() << " state=" << static_cast<int>(truck.get_state())
                  << " location = " << node_type_to_string(truck.get_location())
                  << " timer = " << truck.get_timer()
                  << " dist_to_next_point = " << truck.get_dist_to_next_point()
                  << " health = " << truck.get_health()
                  << " breakdown_count = " << truck.get_breakdown_count()
                  << " dispatched = " << (truck.is_dispatched() ? 1 : 0)
                  << " speed = " << truck.get_speed() << '\n';
    }

    void log_transition(int tick, const Truck& truck, TruckState from_state, TruckState to_state, NodeType at_location) override {
        std::cout << "[tick " << tick << "] "
                  << "truck " << truck.get_id() << " " << static_cast<int>(from_state) << " -> "
                  << static_cast<int>(to_state) << " at " << node_type_to_string(at_location) << '\n';
    }

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
        if (std::string(reason) == "scrapped") {
            std::cout << "[tick " << tick << "] truck " << (truck_index + 1)
                      << " scrapped and removed (breakdown_count>=10)\n";
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

std::unique_ptr<SimulationLogger> build_logger(const SimulationConfig& config) {
    if (config.logging_mode == LoggingMode::CSV) {
        return std::make_unique<CsvSimulationLogger>();
    }
    return std::make_unique<ConsoleSimulationLogger>();
}

std::string generate_truck_id(int sequence_number) {
    // Deterministic 4-letter ID generation: A..Z base-26.
    int value = sequence_number;
    std::string id(4, 'A');
    for (int i = 3; i >= 0; --i) {
        id[i] = static_cast<char>('A' + (value % 26));
        value /= 26;
    }
    return id;
}

void initialize_wear_schedule(std::vector<int>& next_wear_tick, MathModels& math_models, const SimulationConfig& config) {
    for (int i = 0; i < static_cast<int>(next_wear_tick.size()); ++i) {
        next_wear_tick[i] = math_models.sample_uniform_int(config.wear_interval_min, config.wear_interval_max);
    }
}

bool purchasing_enabled(const SimulationConfig& config) {
    return config.purchase_interval_min > 0 &&
           config.purchase_interval_max >= config.purchase_interval_min;
}

void process_pause_resume_controls(
    bool& paused,
    const SimulationConfig& config
) {
    if (!config.enable_pause_resume_controls) {
        return;
    }

#ifdef _WIN32
    while (_kbhit()) {
        const int key = _getch();
        if ((key == 'p' || key == 'P') && !paused) {
            paused = true;
            std::cout << "[control] paused (press 'r' to resume)\n";
            continue;
        }
        if ((key == 'r' || key == 'R') && paused) {
            paused = false;
            std::cout << "[control] resumed\n";
        }
    }
#endif
}

bool at_max_truck_capacity(std::size_t fleet_size, const SimulationConfig& config) {
    if (config.max_truck_count <= 0) {
        return false;
    }
    return static_cast<int>(fleet_size) >= config.max_truck_count;
}

void initialize_purchase_schedule(int& next_purchase_tick, MathModels& math_models, const SimulationConfig& config) {
    if (!purchasing_enabled(config)) {
        next_purchase_tick = -1;
        return;
    }
    next_purchase_tick = math_models.sample_uniform_int(config.purchase_interval_min, config.purchase_interval_max);
}

void purchase_truck_if_due(
    std::vector<Truck>& fleet,
    std::vector<int>& next_wear_tick,
    int tick,
    int& next_purchase_tick,
    MathModels& math_models,
    const SimulationConfig& config
) {
    if (!purchasing_enabled(config) || tick != next_purchase_tick) {
        return;
    }

    if (at_max_truck_capacity(fleet.size(), config)) {
        std::cout << "[tick " << tick << "] purchase skipped (fleet cap reached: "
                  << config.max_truck_count << ")\n";
        next_purchase_tick += math_models.sample_uniform_int(config.purchase_interval_min, config.purchase_interval_max);
        return;
    }

    const std::string new_id = generate_truck_id(static_cast<int>(fleet.size()) + 1);
    fleet.emplace_back(new_id);
    next_wear_tick.push_back(tick + math_models.sample_uniform_int(config.wear_interval_min, config.wear_interval_max));

    std::cout << "[tick " << tick << "] purchased truck " << new_id
              << " (fleet_size=" << fleet.size() << ")\n";

    next_purchase_tick += math_models.sample_uniform_int(config.purchase_interval_min, config.purchase_interval_max);
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

void run_dispatch_if_due(std::vector<Truck>& trucks, int tick, int& next_dispatch_tick, MathModels& math_models, const SimulationConfig& config, SimulationLogger& logger) {
    if (tick != next_dispatch_tick) {
        return;
    }

    const int dispatch_events = math_models.sample_poisson(config.dispatch_lambda);
    if (dispatch_events > 0) {
        int successful_dispatches = 0;
        for (int i = 0; i < dispatch_events; ++i) {
            if (dispatch(trucks)) {
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

bool dispatch(std::vector<Truck>& trucks) {
    if (trucks.empty()) {
        return false;
    }

    int selected_index = -1;
    int best_health = -1;
    for (int i = 0; i < static_cast<int>(trucks.size()); ++i) {
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

    // Only one truck is selected and dispatched per call.
    trucks[selected_index].set_dispatched(true);
    trucks[selected_index].load();
    return true;
}

void simulate(std::vector<Truck>& trucks, int total_timesteps, MathModels& math_models) {
    const SimulationConfig config;
    simulate(trucks, total_timesteps, math_models, config);
}

void simulate(std::vector<Truck>& trucks, int total_timesteps, MathModels& math_models, const SimulationConfig& config) {
    std::unique_ptr<SimulationLogger> logger = build_logger(config);
    simulate(trucks, total_timesteps, math_models, config, *logger);
}

void simulate(std::vector<Truck>& trucks, int total_timesteps, MathModels& math_models, const SimulationConfig& config, SimulationLogger& logger) {
    if (trucks.empty() && !purchasing_enabled(config)) {
        return;
    }

    int next_dispatch_tick = math_models.sample_uniform_int(config.dispatch_interval_min, config.dispatch_interval_max);
    int next_purchase_tick = -1;
    bool paused = false;
    initialize_purchase_schedule(next_purchase_tick, math_models, config);
    std::vector<int> next_wear_tick(static_cast<int>(trucks.size()));
    initialize_wear_schedule(next_wear_tick, math_models, config);
    int simulation_time = 0;

    for (int tick = 1; total_timesteps <= 0 || tick <= total_timesteps; ++tick) {
        process_pause_resume_controls(paused, config);
        while (paused) {
            process_pause_resume_controls(paused, config);
            std::this_thread::sleep_for(std::chrono::milliseconds(config.pause_poll_delay_ms));
        }

        ++simulation_time;
        if (config.logging_mode == LoggingMode::CONSOLE) {
            std::cout << "[time " << simulation_time << " | tick " << tick << "]\n";
        }
        purchase_truck_if_due(trucks, next_wear_tick, tick, next_purchase_tick, math_models, config);

        for (int i = 0; i < static_cast<int>(trucks.size());) {
            if (trucks[i].get_breakdown_count() >= 10) {
                logger.log_breakdown(tick, i, "scrapped");
                trucks.erase(trucks.begin() + i);
                next_wear_tick.erase(next_wear_tick.begin() + i);
                continue;
            }

            const TruckState state_before = trucks[i].get_state();
            const NodeType location_before = trucks[i].get_location();
            apply_wear_if_due(trucks[i], i, tick, next_wear_tick[i], math_models, config, logger);
            apply_health_breakdown_rules(trucks[i], i, tick, logger);
            const bool transitioned_after_breakdown_check =
                (trucks[i].get_state() != state_before || trucks[i].get_location() != location_before);
            if (transitioned_after_breakdown_check) {
                logger.log_transition(tick, trucks[i], state_before, trucks[i].get_state(), trucks[i].get_location());
            }

            if (repair_if_broken(trucks[i])) {
                if (!transitioned_after_breakdown_check &&
                    (trucks[i].get_state() != state_before || trucks[i].get_location() != location_before)) {
                    logger.log_transition(tick, trucks[i], state_before, trucks[i].get_state(), trucks[i].get_location());
                }
                logger.log_state(tick, trucks[i]);
                ++i;
                continue;
            }

            update_speed_for_tick(trucks[i], math_models, config);
            const TruckState state_before_movement = trucks[i].get_state();
            const NodeType location_before_movement = trucks[i].get_location();
            advance_dispatched_truck(trucks[i]);
            if (trucks[i].get_state() != state_before_movement || trucks[i].get_location() != location_before_movement) {
                logger.log_transition(
                    tick,
                    trucks[i],
                    state_before_movement,
                    trucks[i].get_state(),
                    trucks[i].get_location()
                );
            }
            logger.log_state(tick, trucks[i]);
            ++i;
        }

        run_dispatch_if_due(trucks, tick, next_dispatch_tick, math_models, config, logger);
        if (config.logging_mode == LoggingMode::CONSOLE) {
            std::cout << "\n--------------------\n";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(config.tick_delay_ms));
    }
}

void repair_trucks(std::vector<Truck>& trucks) {
    for (int i = 0; i < static_cast<int>(trucks.size()); ++i) {
        if (trucks[i].get_state() == BROKEN) {
            trucks[i].repair();
        }
    }
}