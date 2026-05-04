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

    public:
        Truck(){
            id = 0;
            state = IDLE;
            location = PARKING;
            timer = 0;
        };

        void move();
        void load();
        void unload();
        void break_down();
        void repair();
        void log_state();
        void log_transition(TruckState from, TruckState to);
};

// From	--To--	--Condition--
// IDLE	--HAULING--	Dispatcher assigned route; timers set from graph
// HAULING	--DUMPING--	Travel timer reached 0 at crusher
// DUMPING	--IDLE--	Dump timer reached 0
// --(non-broken)--	--BROKEN--	Stochastic failure
// BROKEN	--IDLE--	Repair timer reached 0