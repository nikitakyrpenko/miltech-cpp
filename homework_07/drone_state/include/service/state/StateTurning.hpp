#pragma once

#include "Calc.hpp"
#include "service/interface/state/IState.hpp"

class StateTurning : public IState {
  static StateTurning instance;

public:
  const static IState* get_instance() { return &instance; }
  const IState* execute(Drone& drone, const Coord& coord, float dt, bool) const override;
  inline std::string name() const override { return "TURNING"; };
};

inline StateTurning StateTurning::instance{};

#include "service/state/StateAccelerating.hpp"

inline const IState* StateTurning::execute(Drone& drone, const Coord& coord, float dt, bool) const
{
  float angle = Calc::calculate_turning_angle(drone.get_position(), coord, drone.get_current_direction());

  if (std::abs(angle) <= drone.get_turn_threshold()) {
    drone.set_current_direction(drone.get_current_direction() + angle);
    return StateAccelerating::get_instance();
  }

  float rot_step = drone.get_angular_speed() * dt;

  if (std::abs(angle) <= rot_step) {
    drone.set_current_direction(drone.get_current_direction() + angle);
    return StateAccelerating::get_instance();
  }

  drone.set_current_direction(drone.get_current_direction() + (angle > 0.0F ? rot_step : -rot_step));
  return StateTurning::get_instance();
}
