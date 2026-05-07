#include <iostream>
#include "mine_graph.h"
#include "math_models.h"
#include "simulate.h"
#include "truck.h"

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

   Truck trucks[] = {Truck(1), Truck(2), Truck(3)};
   MathModels math_models(42);
   simulate(trucks, 3, 120, math_models);
   trucks[0].log_state();
   trucks[1].log_state();
   trucks[2].log_state();
   return 0;
}
