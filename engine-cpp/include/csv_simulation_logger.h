#pragma once

#include "simulation_logger.h"

#include <fstream>
#include <string>

class CsvSimulationLogger final : public SimulationLogger {
   public:
    explicit CsvSimulationLogger(const std::string& output_directory = "../data");
    ~CsvSimulationLogger() override;

    void log_state(int tick, const Truck& truck) override;
    void log_wear(int tick, int truck_index, int wear, int health) override;
    void log_breakdown(int tick, int truck_index, const char* reason) override;
    void log_dispatch(int tick, int events, int calls, int successful) override;
    void log_dispatch_skip(int tick) override;

   private:
    std::ofstream state_stream_;
    std::ofstream event_stream_;
};
