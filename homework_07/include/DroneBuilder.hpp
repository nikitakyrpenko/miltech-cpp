#include "Drone.hpp"

class DroneBuilder {
private:
  Drone drone_;

public:
  DroneBuilder& with_coords(const Coord& pos)
  {
    drone_.position_ = pos;
    return *this;
  }

  DroneBuilder& with_altitude(float altitude)
  {
    drone_.altitude_ = altitude;
    return *this;
  }

  DroneBuilder& with_initial_direction(float init_dir)
  {
    drone_.initial_direction_ = init_dir;
    return *this;
  }

  DroneBuilder& with_attack_speed(float attack_speed)
  {
    drone_.attack_speed_ = attack_speed;
    return *this;
  }

  DroneBuilder& with_acceleration_path(float acceleration_path)
  {
    drone_.acceleration_path_ = acceleration_path;
    return *this;
  }

  DroneBuilder& with_angular_speed(float angular_speed)
  {
    drone_.angular_speed_ = angular_speed;
    return *this;
  }

  DroneBuilder& with_turn_threshold(float turn_threshold)
  {
    drone_.turn_threshold_ = turn_threshold;
    return *this;
  }

  Drone build() { return drone_; }
};