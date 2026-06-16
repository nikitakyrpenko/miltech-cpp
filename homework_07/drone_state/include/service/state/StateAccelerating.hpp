#pragma once

#include "Calc.hpp"
#include "service/interface/state/IState.hpp"

class StateAccelerating : public IState {
  static StateAccelerating instance;

public:
  const static IState* get_instance() { return &instance; }
  const IState* execute(Drone& drone, const Coord& coord, float dt, bool should_decelerate) const override;
  inline std::string name() const override { return "ACCELERATING"; };
};

inline StateAccelerating StateAccelerating::instance{};

#include "service/state/StateDecelerating.hpp"
#include "service/state/StateMoving.hpp"
#include <cmath>

inline const IState* StateAccelerating::execute(Drone& drone, const Coord& coord, float dt, bool should_decelerate) const
{
  drone.set_position(drone.get_position() + Coord{std::cos(drone.get_current_direction()), std::sin(drone.get_current_direction())} *
                                              (drone.get_current_speed() * dt));
  drone.set_current_speed(drone.get_current_speed() + drone.acceleration() * dt);

  float angle = Calc::calculate_turning_angle(drone.get_position(), coord, drone.get_current_direction());
  if (std::abs(angle) > drone.get_turn_threshold())
    return StateDecelerating::get_instance();

  drone.set_current_direction(drone.get_current_direction() + angle);

  if (should_decelerate) {
    float decel_distance = (drone.get_current_speed() * drone.get_current_speed()) / (2.0F * drone.acceleration());
    float distance = Calc::lenght(drone.get_position(), coord);
    if (distance <= decel_distance)
      return StateDecelerating::get_instance();
  }

  if (drone.get_current_speed() >= drone.get_attack_speed()) {
    drone.set_current_speed(drone.get_attack_speed());
    return StateMoving::get_instance();
  }

  return StateAccelerating::get_instance();
}
