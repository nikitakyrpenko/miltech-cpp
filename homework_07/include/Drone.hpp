#pragma once
#include "Coord.hpp"

#include <cstdint>

enum State : std::uint8_t { STOPPED, ACCELERATING, DECELERATING, TURNING, MOVING };

class DroneBuilder;

class Drone {
private:
  Coord position_;
  float altitude_;
  float initial_direction_;
  float attack_speed_;
  float acceleration_path_;
  float angular_speed_;
  float turn_threshold_;
  float current_speed_{0.0F};
  float current_direction_{0.0F};
  State state_{State::STOPPED};

  friend class DroneBuilder;

  Drone() = default;

public:
  static DroneBuilder builder();

  const Coord& getPosition() const;

  float getAltitude() const;
  float getInitialDirection() const;
  float getAttackSpeed() const;
  float getAccelerationPath() const;
  float getAngularSpeed() const;
  float getTurnThreshold() const;
  float getCurrentSpeed() const;
  float getCurrentDirection() const;
  State getState() const;
};