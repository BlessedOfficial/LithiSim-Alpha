#include "truck.h"

#include <iostream>

namespace {

// Until dispatch reads MineGraph edge weights, use the same defaults as main.cpp demo edges.
constexpr int kDefaultDumpSeconds = 5;
constexpr int kDefaultRepairSeconds = 30;

}  // namespace

void Truck::move() {
    if (state != HAULING) {
        return;
    }
    if (timer > 0) {
        --timer;
    }
    if (timer > 0) {
        return;
    }

    const TruckState from = state;
    state = DUMPING;
    location = CRUSHER;
    timer = kDefaultDumpSeconds;
    log_transition(from, state);
}


void Truck::load(int travel_seconds) {
    if (state != IDLE) {
        return;
    }
    if (!dispatched) {
        return;
    }
    // Model: ready at pit (or staging); extend later with explicit travel to PIT.
    if (location != PIT && location != PARKING) {
        return;
    }

    const TruckState from = state;
    state = HAULING;
    timer = travel_seconds > 0 ? travel_seconds : 1;
    log_transition(from, state);
}

void Truck::unload() {
    if (state != DUMPING) {
        return;
    }
    if (timer > 0) {
        --timer;
    }
    if (timer > 0) {
        return;
    }

    const TruckState from = state;
    state = IDLE;
    location = PIT;
    dispatched = false;
    log_transition(from, state);
}

void Truck::break_down() {
    if (state == BROKEN) {
        return;
    }

    const TruckState from = state;
    state = BROKEN;
    location = MAINTENANCE;
    timer = kDefaultRepairSeconds;
    log_transition(from, state);
}

void Truck::repair() {
    if (state != BROKEN) {
        return;
    }
    if (timer > 0) {
        --timer;
    }
    if (timer > 0) {
        return;
    }

    const TruckState from = state;
    state = IDLE;
    location = PARKING;
    dispatched = false;
    log_transition(from, state);
}

void Truck::log_state() {
    std::cout << "truck " << id << " state=" << static_cast<int>(state)
              << " location = " << node_type_to_string(location)
              << " timer = " << timer
              << " health = " << health
              << " dispatched = " << (dispatched ? 1 : 0)
              << " speed = " << speed << '\n';
}

void Truck::log_transition(TruckState from, TruckState to) {
    std::cout << "truck " << id << " " << static_cast<int>(from) << " -> "
              << static_cast<int>(to) << " at " << node_type_to_string(location) << '\n';
}

int Truck::get_health() const {
    return health;
}

void Truck::set_health(int value) {
    if (value < 0) {
        health = 0;
        return;
    }
    if (value > 100) {
        health = 100;
        return;
    }
    health = value;
}

bool Truck::is_dispatched() const {
    return dispatched;
}

void Truck::set_dispatched(bool value) {
    dispatched = value;
}

double Truck::get_speed() const {
    return speed;
}

void Truck::set_speed(double value) {
    if (value <= 0.0) {
        speed = 1.0;
        return;
    }
    speed = value;
}

TruckState Truck::get_state() const {
    return state;
}
