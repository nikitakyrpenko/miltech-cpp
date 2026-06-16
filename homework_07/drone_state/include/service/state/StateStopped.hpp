#pragma once

#include "Calc.hpp"
#include "service/interface/state/IState.hpp"

#include <cmath>

class StateStopped : public IState {
  static StateStopped instance;

public:
  const static IState* get_instance() { return &instance; }
  const IState* execute(Drone& drone, const Coord& coord, float dt, bool) const override;
  inline std::string name() const override { return "STOPPED"; };
};

inline StateStopped StateStopped::instance{};

#include "service/state/StateAccelerating.hpp"
#include "service/state/StateTurning.hpp"

inline const IState* StateStopped::execute(Drone& drone, const Coord& coord, float, bool) const
{
  float angle = Calc::calculate_turning_angle(drone.get_position(), coord, drone.get_current_direction());

  if (std::abs(angle) <= drone.get_turn_threshold()) {
    drone.set_current_direction(drone.get_current_direction() + angle);
    return StateAccelerating::get_instance();
  }

  return StateTurning::get_instance();
}
