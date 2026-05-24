#pragma once
#include "Coord.hpp"

#include <cstdint>

enum State : std::uint8_t { STOPPED, ACCELERATING, DECELERATING, TURNING, MOVING };

class DroneBuilder;

class Drone {
private:
  Coord position_{};
  float altitude_{};
  float initial_direction_{};
  float attack_speed_{};
  float acceleration_path_{};
  float angular_speed_{};
  float turn_threshold_{};
  float current_speed_{0.0F};
  float current_direction_{0.0F};
  State state_{State::STOPPED};

  friend class DroneBuilder;

  Drone() = default;

public:
  static DroneBuilder builder();

  inline float acceleration() const;

  void increment_position(float tick);
  void increment_speed(float tick);
  void increment_direction(float tick);

  float calculate_target_switch_time_penalty(const Coord& target) const;

  const Coord& get_position() const;
  float get_altitude() const;
  float get_initial_direction() const;
  float get_attack_speed() const;
  float get_acceleration_path() const;
  float get_angular_speed() const;
  float get_turn_threshold() const;
  float get_current_speed() const;
  float get_current_direction() const;
  State get_state() const;

private:
  float calculate_drone_target_direction_delta(const Coord& target) const;
};