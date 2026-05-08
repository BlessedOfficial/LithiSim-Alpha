#include "csv_simulation_logger.h"
#include "math_models.h"
#include "simulate.h"
#include "truck.h"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace {

class NullLogger final : public SimulationLogger {
   public:
    void log_state(int, const Truck&) override {}
    void log_transition(int, const Truck&, TruckState, TruckState, NodeType) override {}
    void log_wear(int, int, int, int) override {}
    void log_breakdown(int, int, const char*) override {}
    void log_dispatch(int, int, int, int) override {}
    void log_dispatch_skip(int) override {}
};

void induce_breakdowns(Truck& truck, int count) {
    for (int i = 0; i < count; ++i) {
        truck.break_down();
        while (truck.get_state() == BROKEN) {
            truck.repair();
        }
    }
}

std::string read_file(const std::filesystem::path& path) {
    std::ifstream stream(path);
    REQUIRE(stream.good());
    return std::string(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
}

}  // namespace

TEST_CASE("Truck completes full haul cycle", "[truck]") {
    Truck truck("ABCD");
    truck.set_dispatched(true);
    truck.load();
    REQUIRE(truck.get_state() == HAULING);

    truck.set_speed(6000.0);
    truck.move();
    REQUIRE(truck.get_location() == PIT);
    REQUIRE(truck.get_state() == HAULING);

    truck.set_speed(7000.0);
    truck.move();
    REQUIRE(truck.get_state() == DUMPING);
    REQUIRE(truck.get_location() == CRUSHER);

    for (int i = 0; i < 5; ++i) {
        truck.unload();
    }
    REQUIRE(truck.get_state() == HAULING);
    REQUIRE(truck.get_location() == CRUSHER);

    truck.set_speed(5000.0);
    truck.move();
    REQUIRE(truck.get_state() == IDLE);
    REQUIRE(truck.get_location() == PARKING);
    REQUIRE_FALSE(truck.is_dispatched());
}

TEST_CASE("Simulation purchasing respects max truck cap", "[simulate]") {
    std::vector<Truck> fleet = {Truck("ABCD")};
    MathModels math_models(42);
    SimulationConfig config;
    config.tick_delay_ms = 0;
    config.purchase_interval_min = 1;
    config.purchase_interval_max = 1;
    config.max_truck_count = 3;
    config.dispatch_lambda = 0.01;

    NullLogger logger;
    simulate(fleet, 25, math_models, config, logger);
    REQUIRE(fleet.size() == 3);
}

TEST_CASE("Scrapping removes trucks at 10 breakdowns", "[simulate]") {
    std::vector<Truck> fleet = {Truck("ABCD")};
    induce_breakdowns(fleet[0], 10);
    REQUIRE(fleet[0].get_breakdown_count() >= 10);

    MathModels math_models(42);
    SimulationConfig config;
    config.tick_delay_ms = 0;
    config.purchase_interval_min = 0;
    config.purchase_interval_max = 0;
    config.dispatch_lambda = 0.01;

    NullLogger logger;
    simulate(fleet, 1, math_models, config, logger);
    REQUIRE(fleet.empty());
}

TEST_CASE("CSV logger writes state and event files", "[logger]") {
    const auto temp_root = std::filesystem::temp_directory_path() / "lithisim_logger_test";
    std::filesystem::remove_all(temp_root);
    std::filesystem::create_directories(temp_root);

    {
        CsvSimulationLogger logger(temp_root.string());
        Truck truck("ABCD");
        logger.log_state(1, truck);
        logger.log_wear(1, 0, 2, 98);
        logger.log_dispatch(1, 1, 1, 1);
    }

    const auto state_path = temp_root / "simulation_state_log.csv";
    const auto event_path = temp_root / "simulation_event_log.csv";
    REQUIRE(std::filesystem::exists(state_path));
    REQUIRE(std::filesystem::exists(event_path));

    const std::string state_content = read_file(state_path);
    const std::string event_content = read_file(event_path);
    REQUIRE(state_content.find("tick,truck_id,state,location,timer,dist_to_next_point,health,breakdown_count,dispatched,speed") != std::string::npos);
    REQUIRE(state_content.find("ABCD") != std::string::npos);
    REQUIRE(event_content.find("event_type") != std::string::npos);
    REQUIRE(event_content.find("wear") != std::string::npos);
    REQUIRE(event_content.find("dispatch") != std::string::npos);
}
