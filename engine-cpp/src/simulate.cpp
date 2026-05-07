#include "simulate.h"

#include <algorithm>
#include <cmath>
#include <iostream>

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
    if (truck_count <= 0 || total_timesteps <= 0) {
        return;
    }

    int next_dispatch_tick = math_models.sample_uniform_int(20, 25);
    constexpr double kDispatchLambda = 0.03;

    for (int tick = 1; tick <= total_timesteps; ++tick) {
        for (int i = 0; i < truck_count; ++i) {
            // Only dispatched trucks are allowed to progress through cycle operations.
            if (!trucks[i].is_dispatched()) {
                continue;
            }
            trucks[i].repair();
            trucks[i].move();
            trucks[i].unload();
        }

        if (tick == next_dispatch_tick) {
            const int dispatch_events = math_models.sample_poisson(kDispatchLambda);
            if (dispatch_events > 0) {
                int successful_dispatches = 0;
                for (int i = 0; i < dispatch_events; ++i) {
                    if (dispatch(trucks, truck_count, math_models)) {
                        ++successful_dispatches;
                    }
                }
                std::cout << "[tick " << tick << "] dispatch triggered (events=" << dispatch_events
                          << ", calls=" << dispatch_events
                          << ", successful=" << successful_dispatches << ")\n";
            } else {
                std::cout << "[tick " << tick << "] dispatch skipped (events=0)\n";
            }
            next_dispatch_tick += math_models.sample_uniform_int(20, 25);
        }
    }
}
