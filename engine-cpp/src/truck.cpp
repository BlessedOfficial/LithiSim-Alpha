#include "truck.h"

#include <cctype>
#include <iostream>
#include <random>

namespace {

// Until dispatch reads MineGraph edge weights, use the same defaults as main.cpp demo edges.
constexpr int kDefaultDumpSeconds = 5;
constexpr int kRepairSecondsMin = 20;
constexpr int kRepairSecondsMax = 45;
constexpr int kRepairedHealthMin = 95;
constexpr int kRepairedHealthMax = 100;

MineGraph build_default_route_graph() {
    MineGraph graph;
    graph.add_edge(PARKING, PIT, 1000);
    graph.add_edge(PIT, CRUSHER, 1200);
    graph.add_edge(CRUSHER, PARKING, 800);
    return graph;
}

const MineGraph kRouteGraph = build_default_route_graph();

double edge_distance(NodeType from, NodeType to) {
    return static_cast<double>(kRouteGraph.weights[static_cast<int>(from)][static_cast<int>(to)]);
}

int sample_uniform_int(int min_value, int max_value) {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> distribution(min_value, max_value);
    return distribution(rng);
}

}  // namespace

std::string Truck::normalize_id(const std::string& raw_id) {
    std::string cleaned;
    cleaned.reserve(4);
    for (char ch : raw_id) {
        if (std::isalpha(static_cast<unsigned char>(ch))) {
            cleaned.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(ch))));
            if (cleaned.size() == 4) {
                break;
            }
        }
    }
    while (cleaned.size() < 4) {
        cleaned.push_back('X');
    }
    return cleaned;
}

Truck::Truck(const std::string& id)
    : id(normalize_id(id)),
      state(IDLE),
      location(PARKING),
      timer(0),
      dist_to_next_point(0),
      health(100),
      dispatched(false),
      speed(22.0) {}

void Truck::move() {
    if (state != HAULING) {
        return;
    }

    if (dist_to_next_point <= 0) {
        if (location == PARKING) {
            dist_to_next_point = edge_distance(PARKING, PIT);
        } else if (location == PIT) {
            dist_to_next_point = edge_distance(PIT, CRUSHER);
        } else if (location == CRUSHER) {
            dist_to_next_point = edge_distance(CRUSHER, PARKING);
        }
    }

    if (dist_to_next_point <= 0) {
        // Defensive fallback if an edge weight is missing from the route graph.
        dist_to_next_point = speed;
    }

    dist_to_next_point -= speed;
    if (dist_to_next_point > 0) {
        return;
    }

    if (location == PARKING) {
        location = PIT;
        dist_to_next_point = edge_distance(PIT, CRUSHER);
        if (dist_to_next_point <= 0) {
            dist_to_next_point = speed;
        }
        return;
    }

    if (location == PIT) {
        const TruckState from = state;
        state = DUMPING;
        location = CRUSHER;
        timer = kDefaultDumpSeconds;
        dist_to_next_point = 0;
        log_transition(from, state);
        return;
    }

    if (location == CRUSHER) {
        const TruckState from = state;
        state = IDLE;
        location = PARKING;
        dispatched = false;
        dist_to_next_point = 0;
        log_transition(from, state);
    }
}


void Truck::load(int travel_seconds) {
    (void)travel_seconds;
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
    timer = 0;
    dist_to_next_point = edge_distance(PARKING, PIT);
    if (dist_to_next_point <= 0) {
        dist_to_next_point = speed;
    }
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
    state = HAULING;
    dist_to_next_point = edge_distance(CRUSHER, PARKING);
    if (dist_to_next_point <= 0) {
        dist_to_next_point = speed;
    }
    log_transition(from, state);
}

void Truck::break_down() {
    if (state == BROKEN) {
        return;
    }

    const TruckState from = state;
    state = BROKEN;
    location = MAINTENANCE;
    timer = sample_uniform_int(kRepairSecondsMin, kRepairSecondsMax);
    dist_to_next_point = 0;
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
    dist_to_next_point = 0;
    set_health(sample_uniform_int(kRepairedHealthMin, kRepairedHealthMax));
    log_transition(from, state);
}

void Truck::log_state() const {
    std::cout << "truck " << id << " state=" << static_cast<int>(state)
              << " location = " << node_type_to_string(location)
              << " timer = " << timer
              << " dist_to_next_point = " << dist_to_next_point
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
    }else if(state == BROKEN || state == IDLE)
    {
        speed = 0.0;
        return;
    }
    speed = value;
}

TruckState Truck::get_state() const {
    return state;
}

const std::string& Truck::get_id() const {
    return id;
}

NodeType Truck::get_location() const {
    return location;
}

int Truck::get_timer() const {
    return timer;
}

double Truck::get_dist_to_next_point() const {
    return dist_to_next_point;
}
