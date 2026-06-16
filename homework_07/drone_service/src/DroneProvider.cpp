#include "service/DroneProvider.hpp"

#include "Calc.hpp"
#include "dto/AmmoDTO.hpp"
#include "models/Ammo.hpp"
#include "models/Coord.hpp"
#include "models/Drone.hpp"
#include "models/DroneBuilder.hpp"
#include "models/Simulation.hpp"
#include "models/Task.hpp"
#include "service/interfaces/IConfigLoader.hpp"

#include <algorithm>
#include <memory>
#include <stdexcept>

namespace {

Drone create_drone(const ConfigDTO& dto)
{
  return Drone::builder()
    .with_coords(dto.position_)
    .with_altitude(dto.altitude_)
    .with_acceleration_path(dto.acceleration_path_)
    .with_attack_speed(dto.attack_speed_)
    .with_initial_direction(dto.initial_direction_)
    .with_turn_threshold(dto.turn_threshold_)
    .with_angular_speed(dto.angular_speed_)
    .build();
}

Ammo create_ammo(const ConfigDTO& dto, const AmmoDTO& arsenal)
{
  auto it = std::ranges::find_if(arsenal.ammos_, [&dto](const AmmoDTO::Ammo& a) { return a.name == dto.ammo_; });

  if (it == arsenal.ammos_.end()) {
    throw std::runtime_error("cannot dispatch ammo");
  }

  return Ammo(it->name, it->mass, it->drag, it->lift);
}

Simulation create_simulation(const ConfigDTO& dto)
{
  return Simulation{dto.time_step_, dto.hit_radius_, dto.target_array_timestep_};
}
}  // namespace

DroneProvider::DroneProvider(const std::shared_ptr<IConfigLoader> config)
  : drone_(create_drone(config->get_config()))
  , ammo_(create_ammo(config->get_config(), config->get_arsenal()))
  , simulation_(create_simulation(config->get_config()))
  , previous_position_(drone_.get_position())
{
}

const Coord DroneProvider::get_active_coord() const
{
  if (visited_intermidiate_ || !has_task_contains_intermidiate()) {
    return task_.solution_.fire_;
  }
  return *task_.solution_.intermididate_;
}

const Drone& DroneProvider::execute(const Task& proposal, float dt)
{
  if (task_.id_ == -1 || task_.id_ != proposal.id_) {
    task_ = proposal;
    visited_intermidiate_ = false;
  }
  else {
    task_.solution_ = proposal.solution_;
  }

  Coord active = get_active_coord();

  // check in terms of current task drone visited intermididate coord
  if (has_task_contains_intermidiate() && !visited_intermidiate_) {
    if (Calc::point_to_segment_distance(*task_.solution_.intermididate_, previous_position_, drone_.get_position()) <=
        simulation_.hit_radius_) {
      visited_intermidiate_ = true;
      active = get_active_coord();
    }
  }

  previous_position_ = drone_.get_position();

  drone_state_ = drone_state_->execute(drone_, active, dt, has_task_contains_intermidiate() && !visited_intermidiate_);

  return this->get_drone();
}

bool DroneProvider::has_task_completed() const
{
  if (task_.id_ == -1) {
    return false;
  }
  if (visited_intermidiate_ || !has_task_contains_intermidiate()) {
    const float release_tolerance = drone_.get_attack_speed() * simulation_.time_step_;
    return Calc::point_to_segment_distance(task_.solution_.fire_, previous_position_, drone_.get_position()) <= release_tolerance;
  }
  return false;
}

DroneProvider::~DroneProvider(){};
