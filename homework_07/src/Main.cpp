#include "models/SimulationStep.hpp"
#include "service/MissionFactory.hpp"

#include "json.hpp"

#include <exception>
#include <fstream>
#include <iostream>
#include <latch>
#include <vector>

static constexpr int MAX_ITERATIONS = 10000;

static void dump_json(const std::vector<SimulationStep>& steps)
{
  nlohmann::ordered_json j;
  j["totalSteps"] = steps.size();
  j["steps"] = nlohmann::json::array();

  for (const SimulationStep& s : steps) {
    j["steps"].push_back({
      {"targetIndex", s.target_id_},
      {"direction", s.direction_},
      {"state", s.state_},
      {"position", {{"x", s.position_.x_}, {"y", s.position_.y_}}},
      {"dropPoint", {{"x", s.drop_point_.x_}, {"y", s.drop_point_.y_}}},
      {"aimPoint", {{"x", s.aim_point_.x_}, {"y", s.aim_point_.y_}}},
      {"predictedTarget", {{"x", s.predicted_target_.x_}, {"y", s.predicted_target_.y_}}},
      {"timeSecSinceStart", s.elapsed_},
    });
  }

  std::ofstream ofs("simulation.json");
  ofs << j.dump(2);
}

int main(int argc, char* argv[])
{
  if (argc < 4) {
    std::cerr << "Usage: " << argv[0] << " <config.json> <ammo.json> <targets.json>" << std::endl;
    return 1;
  }

  MissionFactory factory;

  SimulationBundle sim;
  try {
    sim = factory.create(LoaderType::JSON, SolverType::TABLE, argv[1], argv[2], argv[3], argv[4]);
  }
  catch (const std::exception& e) {
    std::cerr << "Failed to create mission: " << e.what() << std::endl;
    return 1;
  }

  std::latch latch{3};
  sim.target->start(latch);
  sim.physics->start(latch);
  sim.mission->start(latch, MAX_ITERATIONS);

  sim.mission->join();  // blocks until the drone reaches the fire point

  sim.target->interrupt();
  sim.physics->interrupt();

  const std::vector<SimulationStep>& steps = sim.mission->get_steps();
  dump_json(steps);

  std::cout << "Wrote " << steps.size() << " steps to simulation.json" << std::endl;
  return 0;
}
