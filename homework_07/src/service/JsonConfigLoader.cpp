
#include "json.hpp"
#include "service/JsonConfigLoader.hpp"
#include "models/DroneBuilder.hpp"

#include <fstream>
#include <iostream>
#include <string>

JsonConfigLoader* JsonConfigLoader::create(const char* sim, const char* ammo, const char* target)
{
  ConfigContext* config = load_config(sim);
  if (!config)
    return nullptr;

  AmmoContext* arsenal = load_arsenal(ammo);
  if (!arsenal) {
    delete config;
    return nullptr;
  }

  TargetContext* targets = load_targets(target, static_cast<int>(config->target_array_timestep_));
  if (!targets) {
    delete config;
    delete arsenal;
    return nullptr;
  }

  return new JsonConfigLoader(config, arsenal, targets);
}

ConfigContext* JsonConfigLoader::load_config(const char* source)
{
  std::ifstream simulation(source);

  if (!simulation.is_open()) {
    std::cerr << "File not found : " << source << std::endl;
    return nullptr;
  }

  nlohmann::json j;
  simulation >> j;

  Drone* drone{nullptr};

  std::string ammo{};

  float time_step_{};
  float hit_radius_{};
  float array_time_step{};

  try {
    drone = new Drone(DroneBuilder::from_json(j["drone"]));

    ammo = j.at("ammo").get<std::string>();
    array_time_step = j.at("targetArrayTimeStep").get<float>();
    time_step_ = j["simulation"]["timestep"].get<float>();
    hit_radius_ = j["simulation"]["hitRadius"].get<float>();
  }
  catch (const nlohmann::json::exception& e) {
    std::cerr << e.what() << std::endl;
    delete drone;
    return nullptr;
  }
  return new ConfigContext{drone, ammo, array_time_step, time_step_, hit_radius_};
}

AmmoContext* JsonConfigLoader::load_arsenal(const char* source)
{
  std::ifstream arsenal(source);

  if (!arsenal.is_open()) {
    std::cerr << "File not found : " << source << std::endl;
    return nullptr;
  }

  nlohmann::json j;
  int ammo_count{};
  Ammo** result{nullptr};
  try {
    arsenal >> j;
    ammo_count = j.size();
    result = new Ammo*[ammo_count];

    for (int i = 0; i < ammo_count; i++) {
      result[i] =
        new Ammo{j[i]["name"].get<std::string>(), j[i]["mass"].get<float>(), j[i]["drag"].get<float>(), j[i]["lift"].get<float>()};
    }
  }
  catch (const nlohmann::json::exception& e) {
    std::cerr << e.what() << std::endl;
    return nullptr;
  }
  return new AmmoContext{result, ammo_count};
}

TargetContext* JsonConfigLoader::load_targets(const char* source, int array_time_step)
{
  std::ifstream file(source);

  if (!file.is_open()) {
    std::cerr << "File not found : " << source << std::endl;
    return nullptr;
  }

  nlohmann::json j;
  file >> j;

  try {
    int target_count = j.at("targetCount").get<int>();
    int time_steps = j.at("timeSteps").get<int>();

    Target** targets = new Target*[target_count];

    for (int i = 0; i < target_count; i++) {
      Coord* coords = new Coord[time_steps];
      for (int k = 0; k < time_steps; k++) {
        coords[k].x_ = j["targets"][i]["positions"][k]["x"].get<float>();
        coords[k].y_ = j["targets"][i]["positions"][k]["y"].get<float>();
      }
      targets[i] = new Target(i, coords);
    }

    return new TargetContext{targets, target_count, time_steps, array_time_step};
  }
  catch (const nlohmann::json::exception& e) {
    std::cerr << e.what() << std::endl;
    return nullptr;
  }
}

JsonConfigLoader::~JsonConfigLoader()
{
  delete config_->drone_;
  delete config_;

  for (int i = 0; i < ammo_->size; i++) {
    delete ammo_->ammos_[i];
  }
  delete[] ammo_->ammos_;
  delete ammo_;

  for (int i = 0; i < target_->target_count_; i++) {
    delete target_->targets_[i];
  }
  delete[] target_->targets_;
  delete target_;
}
