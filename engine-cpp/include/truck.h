#pragma once

#include "mine_graph.h"

enum TruckState {
    IDLE,
    HAULING,
    DUMPING,
    BROKEN
};

class Truck {
   
   private:
        int id;
        TruckState state;
        NodeType location;
        int timer;
        int health;
        bool dispatched;
        double speed;

    public:
        Truck(int id){
            this->id = id;
            state = IDLE;
            location = PARKING;
            timer = 0;
            health = 100;
            dispatched = false;
            speed = 22.0;
        };
        
       
        void move();
        void load(int travel_seconds);
        void unload();
        void break_down();
        void repair();
        void log_state();
        void log_transition(TruckState from, TruckState to);
        int get_health() const;
        void set_health(int value);
        bool is_dispatched() const;
        void set_dispatched(bool value);
        double get_speed() const;
        void set_speed(double value);
        TruckState get_state() const;
};

// From	--To--	--Condition--
// IDLE	--HAULING--	Dispatcher assigned route; timers set from graph
// HAULING	--DUMPING--	Travel timer reached 0 at crusher
// DUMPING	--IDLE--	Dump timer reached 0
// --(non-broken)--	--BROKEN--	Stochastic failure
// BROKEN	--IDLE--	Repair timer reached 0