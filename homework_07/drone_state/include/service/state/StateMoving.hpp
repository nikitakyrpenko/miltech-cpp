#pragma once

#include "service/interface/state/IState.hpp"

class StateMoving : public IState {
  static StateMoving instance;

public:
  const static IState* get_instance() { return &instance; }
  const IState* execute(Drone& drone, const Coord& coord, float dt, bool) const override;
  inline std::string name() const override { return "MOVING"; };
};

inline StateMoving StateMoving::instance{};

#include "Calc.hpp"
#include "service/state/StateDecelerating.hpp"
#include <cmath>

inline const IState* StateMoving::execute(Drone& drone, const Coord& coord, float dt, bool should_decelerate) const
{
  drone.set_position(drone.get_position() + Coord{std::cos(drone.get_current_direction()), std::sin(drone.get_current_direction())} *
                                              (drone.get_current_speed() * dt));
  drone.set_current_speed(drone.get_attack_speed());

  float angle = Calc::calculate_turning_angle(drone.get_position(), coord, drone.get_current_direction());

  if (std::abs(angle) > drone.get_turn_threshold()) {
    return StateDecelerating::get_instance();
  }

  drone.set_current_direction(drone.get_current_direction() + angle);

  if (should_decelerate) {
    float distance = Calc::lenght(drone.get_position(), coord);
    if (distance <= drone.get_acceleration_path()) {
      return StateDecelerating::get_instance();
    }
  }

  return StateMoving::get_instance();
}
