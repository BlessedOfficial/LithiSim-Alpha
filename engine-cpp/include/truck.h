#pragma once

#include "mine_graph.h"
#include <string>

enum TruckState {
    IDLE,
    HAULING,
    DUMPING,
    BROKEN
};

class Truck {
   
   private:
        std::string id;
        TruckState state;
        NodeType location;
        int timer;
        double dist_to_next_point;
        int health;
        int breakdown_count;
        bool dispatched;
        double speed;
        static std::string normalize_id(const std::string& raw_id);

    public:
        explicit Truck(const std::string& id);
        
       
        void move();
        void load();
        void unload();
        void break_down();
        void repair();
        int get_health() const;
        void set_health(int value);
        bool is_dispatched() const;
        void set_dispatched(bool value);
        double get_speed() const;
        void set_speed(double value);
        TruckState get_state() const;
        const std::string& get_id() const;
        NodeType get_location() const;
        int get_timer() const;
        double get_dist_to_next_point() const;
        int get_breakdown_count() const;
};

// From	--To--	--Condition--
// IDLE	--HAULING--	Dispatcher assigned route; timers set from graph
// HAULING	--DUMPING--	Travel timer reached 0 at crusher
// DUMPING	--IDLE--	Dump timer reached 0
// --(non-broken)--	--BROKEN--	Stochastic failure
// BROKEN	--IDLE--	Repair timer reached 0