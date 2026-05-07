#pragma once

#include "math_models.h"
#include "truck.h"

bool dispatch(Truck trucks[], int truck_count, MathModels& math_models);
void simulate(Truck trucks[], int truck_count, int total_timesteps, MathModels& math_models);
