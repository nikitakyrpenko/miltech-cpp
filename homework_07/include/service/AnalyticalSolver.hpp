#pragma once

#include "models/Coord.hpp"
#include "models/Drone.hpp"
#include "service/interface/IBallisticSolver.hpp"

class AnalyticalSolver : public IBallisticSolver {
public:
  Task solve(const Drone& drone, const Ammo& ammo, const Coord& target, float tick) override;
  ~AnalyticalSolver() override;

private:
  float calculate_time_to_reach(
    const Coord& from, const Coord& to, float current_speed, float attack_speed, float acceleration, bool decelerateInDest);
  float calculate_time_to_turn(const Coord& from, const Coord& to, float current_direction, float turn_threshold, float angular_speed);
};
