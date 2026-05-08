#pragma once

#include "truck.h"

class SimulationLogger {
   public:
    virtual ~SimulationLogger() = default;

    virtual void log_state(int tick, const Truck& truck) = 0;
    virtual void log_transition(int tick, const Truck& truck, TruckState from_state, TruckState to_state, NodeType at_location) = 0;
    virtual void log_wear(int tick, int truck_index, int wear, int health) = 0;
    virtual void log_breakdown(int tick, int truck_index, const char* reason) = 0;
    virtual void log_dispatch(int tick, int events, int calls, int successful) = 0;
    virtual void log_dispatch_skip(int tick) = 0;
};
