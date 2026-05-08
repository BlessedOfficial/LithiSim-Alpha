#include "csv_simulation_logger.h"

#include <filesystem>

CsvSimulationLogger::CsvSimulationLogger(const std::string& output_directory) {
    const std::filesystem::path dir(output_directory);
    std::filesystem::create_directories(dir);

    const std::filesystem::path state_path = dir / "simulation_state_log.csv";
    const std::filesystem::path event_path = dir / "simulation_event_log.csv";

    state_stream_.open(state_path, std::ios::out | std::ios::trunc);
    event_stream_.open(event_path, std::ios::out | std::ios::trunc);

    state_stream_ << "tick,truck_id,state,location,timer,dist_to_next_point,health,dispatched,speed\n";
    event_stream_ << "tick,event_type,truck_id,truck_index,wear,health,dispatch_events,dispatch_calls,dispatch_successful,reason\n";
}

CsvSimulationLogger::~CsvSimulationLogger() = default;

void CsvSimulationLogger::log_state(int tick, const Truck& truck) {
    state_stream_ << tick << ","
            << truck.get_id() << ","
            << static_cast<int>(truck.get_state()) << ","
            << node_type_to_string(truck.get_location()) << ","
            << truck.get_timer() << ","
            << truck.get_dist_to_next_point() << ","
            << truck.get_health() << ","
            << (truck.is_dispatched() ? 1 : 0) << ","
            << truck.get_speed() << "\n";
}

void CsvSimulationLogger::log_wear(int tick, int truck_index, int wear, int health) {
    event_stream_ << tick << ",wear,,"
                  << (truck_index + 1) << ","
                  << wear << ","
                  << health << ",,,,,\n";
}

void CsvSimulationLogger::log_breakdown(int tick, int truck_index, const char* reason) {
    event_stream_ << tick << ",breakdown,,"
                  << (truck_index + 1) << ",,,,,,,,"
                  << reason << "\n";
}

void CsvSimulationLogger::log_dispatch(int tick, int events, int calls, int successful) {
    event_stream_ << tick << ",dispatch,,,,,"
                  << events << ","
                  << calls << ","
                  << successful << ",\n";
}

void CsvSimulationLogger::log_dispatch_skip(int tick) {
    event_stream_ << tick << ",dispatch_skip,,,,,"
                  << 0 << ",,,\n";
}
