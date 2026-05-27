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

  Drone(){};

public:
  static DroneBuilder builder();

  inline float acceleration() const { return (attack_speed_ * attack_speed_) / (2.0F * acceleration_path_); }

  const Coord& get_position() const { return position_; }
  float get_altitude() const { return altitude_; }
  float get_initial_direction() const { return initial_direction_; }
  float get_attack_speed() const { return attack_speed_; }
  float get_acceleration_path() const { return acceleration_path_; }
  float get_angular_speed() const { return angular_speed_; }
  float get_turn_threshold() const { return turn_threshold_; }
  float get_current_speed() const { return current_speed_; }
  float get_current_direction() const { return current_direction_; }
  State get_state() const { return state_; }

  void set_position(const Coord& position) { position_ = position; }
  void set_current_speed(float speed) { current_speed_ = speed; }
  void set_current_direction(float direction) { current_direction_ = direction; }
  void set_state(State state) { state_ = state; }

  void increment_speed(float dt);
  void increment_direction(const Coord& target, float dt);
  void increment_position(float dt);
  bool is_position_reached(const Coord& target, float threshold) const;
  float penalty(const Coord& target) const;

private:
  float calculate_drone_target_direction_delta(const Coord& target) const;
};
