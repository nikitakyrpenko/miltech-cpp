#include "service/AnalyticalSolver.hpp"
#include "Calc.hpp"
#include "models/Coord.hpp"

float AnalyticalSolver::calculate_time_to_turn(
  const Coord& from, const Coord& to, float current_direction, float turn_threshold, float angular_speed) const
{
  float turning_angle = std::abs(Calc::calculate_turning_angle(from, to, current_direction));
  if (turning_angle <= turn_threshold) {
    return 0.0F;
  }
  return turning_angle / angular_speed;
}

float AnalyticalSolver::calculate_time_to_reach(
  const Coord& from, const Coord& to, float current_speed, float attack_speed, float acceleration, bool decelerate_in_dest) const
{
  float distance = Calc::lenght(from, to);
  float remaining_acceleration_distance = (attack_speed * attack_speed - current_speed * current_speed) / (2.0F * acceleration);

  // case when full stop in the dest. point is not needed
  if (!decelerate_in_dest) {
    if (distance < remaining_acceleration_distance) {
      return (-current_speed + std::sqrt(current_speed * current_speed + 2.0F * acceleration * distance)) / acceleration;
    }

    float time_to_accelerate = (attack_speed - current_speed) / acceleration;
    float time_to_cruise = (distance - remaining_acceleration_distance) / attack_speed;

    return time_to_accelerate + time_to_cruise;
  }
  float distance_to_accelerate = (attack_speed * attack_speed - current_speed * current_speed) / (2.0F * acceleration);
  float distance_to_decelerate = (attack_speed * attack_speed) / (2.0F * acceleration);

  // trapezoid case
  if (distance >= distance_to_accelerate + distance_to_decelerate) {
    float time_to_accelerate = (attack_speed - current_speed) / acceleration;
    float time_to_decelerate = attack_speed / acceleration;
    float time_to_cruise = (distance - distance_to_accelerate - distance_to_decelerate) / attack_speed;

    return time_to_accelerate + time_to_cruise + time_to_decelerate;
  }

  // triangle case
  float velocity_peak = std::sqrt(acceleration * distance + (current_speed * current_speed) / 2.0F);
  float time_to_accelerate = (velocity_peak - current_speed) / acceleration;
  float time_to_decelerate = velocity_peak / acceleration;

  return time_to_decelerate + time_to_accelerate;
}

Task AnalyticalSolver::solve(const Drone& drone, const Ammo& ammo, const Coord& target) const
{
  float ammo_time_to_fall = ammo.calculate_ammo_time_to_fall(drone.get_altitude(), drone.get_attack_speed());
  float ammo_distance_to_fall = ammo.calculate_ammo_distance_to_fall(ammo_time_to_fall, drone.get_attack_speed());

  float distance = Calc::lenght(target, drone.get_position());

  // no intermidiate point needed
  if (ammo_distance_to_fall + drone.get_acceleration_path() < distance) {
    float ratio = (distance - ammo_distance_to_fall) / distance;

    Coord fire{(drone.get_position() + ((target - drone.get_position()) * ratio))};

    float time_to_turn = calculate_time_to_turn(
      drone.get_position(), fire, drone.get_current_direction(), drone.get_turn_threshold(), drone.get_angular_speed());

    // TODO : if snap turn may be performed current_speed need to be recalculatated
    float time_to_reach =
      calculate_time_to_reach(drone.get_position(), fire, drone.get_current_speed(), drone.get_attack_speed(), drone.acceleration(), false);

    return {
      .intermidiate_ = fire,
      .fire_ = fire,
      .has_intermidiate_ = false,
      .time_taken = time_to_turn + time_to_reach,
    };
  }

  // intermidiate point needed
  Coord intermidiate{};

  float intermediate_ratio = (ammo_distance_to_fall + drone.get_acceleration_path()) / distance;

  // distance between target and drone is zero, or drone is already within range -> use drone position
  if (distance == 0.0F || intermediate_ratio >= 1.0F) {
    intermidiate = drone.get_position();
  }
  else {
    intermidiate = target - (target - drone.get_position()) * intermediate_ratio;
  }

  float distance_from_intermidiate_to_target = Calc::lenght(intermidiate, target);
  float ratio = (distance_from_intermidiate_to_target - ammo_distance_to_fall) / distance_from_intermidiate_to_target;

  Coord fire{intermidiate + (target - intermidiate) * ratio};

  float current_to_intermidiate_time_to_turn = calculate_time_to_turn(
    drone.get_position(), intermidiate, drone.get_current_direction(), drone.get_turn_threshold(), drone.get_angular_speed());

  float intermidiate_to_fire_time_to_turn = calculate_time_to_turn(
    intermidiate, fire, Calc::angle(drone.get_position(), intermidiate), drone.get_turn_threshold(), drone.get_angular_speed());

  float current_to_intermidiate_time_to_reach = calculate_time_to_reach(
    drone.get_position(), intermidiate, drone.get_current_speed(), drone.get_attack_speed(), drone.acceleration(), false);

  // TODO : if snap turn may be performed current_speed need to be recalculatated
  float intermidiate_to_fire_time_to_reach =
    calculate_time_to_reach(intermidiate, fire, 0.0F, drone.get_attack_speed(), drone.acceleration(), false);

  return {.intermidiate_ = intermidiate,
          .fire_ = fire,
          .has_intermidiate_ = true,
          .time_taken = current_to_intermidiate_time_to_turn + intermidiate_to_fire_time_to_turn + current_to_intermidiate_time_to_reach +
                        intermidiate_to_fire_time_to_reach};
}

AnalyticalSolver::~AnalyticalSolver(){};