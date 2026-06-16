#include "service/FirepointProvider.hpp"
#include "models/Task.hpp"
#include "Calc.hpp"

#include <optional>

const BallisticSolution FirepointProvider::solve(const Drone& drone, const FallResult& fall, const Coord& target) const
{
  float ammo_distance_to_fall = fall.distance;

  float distance = Calc::lenght(target, drone.get_position());

  float remaining_acceleration_distance =
    (drone.get_attack_speed() * drone.get_attack_speed() - drone.get_current_speed() * drone.get_current_speed()) /
    (2.0F * drone.acceleration());

  if (ammo_distance_to_fall + remaining_acceleration_distance < distance) {
    float ratio = (distance - ammo_distance_to_fall) / distance;

    return {(drone.get_position() + ((target - drone.get_position()) * ratio)), std::nullopt};
  }

  if (distance == 0.0F) {
    return {drone.get_position(), std::nullopt};
  }

  float intermediate_ratio = (ammo_distance_to_fall + drone.get_acceleration_path()) / distance;

  if (intermediate_ratio >= 1.0F) {
    float direct_ratio = (distance - ammo_distance_to_fall) / distance;
    return {drone.get_position() + (target - drone.get_position()) * direct_ratio, std::nullopt};
  }

  Coord intermidiate = target - (target - drone.get_position()) * intermediate_ratio;

  float distance_from_intermidiate_to_target = Calc::lenght(intermidiate, target);
  float ratio = (distance_from_intermidiate_to_target - ammo_distance_to_fall) / distance_from_intermidiate_to_target;

  Coord fire{intermidiate + (target - intermidiate) * ratio};

  return {fire, intermidiate};
}

FirepointProvider::~FirepointProvider() {}