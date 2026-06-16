#pragma once

#include "models/Ammo.hpp"
#include "models/Drone.hpp"
#include "models/Simulation.hpp"
#include "models/Task.hpp"

class IDroneProvider {
public:
  virtual const Drone& get_drone() const = 0;
  virtual const Ammo& get_ammo() const = 0;
  virtual const Simulation& get_simulation() const = 0;

  virtual std::string get_current_state() const = 0;
  virtual const Coord get_active_coord() const = 0;

  virtual const Drone& execute(const Task& task, float dt) = 0;

  virtual bool has_task_completed() const = 0;
  virtual bool has_task_contains_intermidiate() const = 0;

  virtual ~IDroneProvider() = default;
};
