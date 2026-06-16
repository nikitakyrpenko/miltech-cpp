#pragma once

#include "service/interface/state/IState.hpp"

class StateDecelerating : public IState {
  static StateDecelerating instance;

public:
  const static IState* get_instance() { return &instance; }
  const IState* execute(Drone& drone, const Coord& coord, float dt, bool) const override;
  inline std::string name() const override { return "DECELERATING"; };
};

inline StateDecelerating StateDecelerating::instance{};

#include "Calc.hpp"
#include "service/state/StateAccelerating.hpp"
#include "service/state/StateStopped.hpp"

#include <cmath>

inline const IState* StateDecelerating::execute(Drone& drone, const Coord& coord, float dt, bool) const
{
  drone.set_position(drone.get_position() + Coord{std::cos(drone.get_current_direction()), std::sin(drone.get_current_direction())} *
                                              (drone.get_current_speed() * dt));
  drone.set_current_speed(drone.get_current_speed() - drone.acceleration() * dt);

  float angle = Calc::calculate_turning_angle(drone.get_position(), coord, drone.get_current_direction());
  if (std::abs(angle) <= drone.get_turn_threshold())
    drone.set_current_direction(drone.get_current_direction() + angle);

  if (drone.get_current_speed() <= 0.0F) {
    drone.set_current_speed(0.0F);
    return StateStopped::get_instance();
  }

  // decelerating to reorient, not to arrive -> keep slowing down regardless of distance to target
  if (std::abs(angle) > drone.get_turn_threshold())
    return StateDecelerating::get_instance();

  float decel_distance = (drone.get_current_speed() * drone.get_current_speed()) / (2.0F * drone.acceleration());
  float distance = Calc::lenght(drone.get_position(), coord);
  if (distance > decel_distance)
    return StateAccelerating::get_instance();

  return StateDecelerating::get_instance();
}
