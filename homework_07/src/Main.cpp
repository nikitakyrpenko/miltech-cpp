#include "GpioSignal.hpp"
#include "models/SimulationStep.hpp"
#include "service/MissionFactory.hpp"

#include "json.hpp"

#include <chrono>
#include <exception>
#include <fstream>
#include <iostream>
#include <latch>
#include <string>
#include <vector>

static constexpr int MAX_ITERATIONS = 10000;
static constexpr std::chrono::milliseconds MISSION_DONE_PULSE{100};
static constexpr const char* BALLISTIC_TABLE_PATH = "homework_07/data/ballistic_table.txt";

static void dump_json(const std::vector<SimulationStep>& steps)
{
  nlohmann::ordered_json j;
  j["totalSteps"] = steps.size();
  j["steps"] = nlohmann::json::array();

  for (const SimulationStep& s : steps) {
    nlohmann::ordered_json targets = nlohmann::json::array();
    for (const Target& t : s.all_targets_) {
      targets.push_back({
        {"id", t.target_id_},
        {"x", t.pos_.x_},
        {"y", t.pos_.y_},
      });
    }

    j["steps"].push_back({
      {"targetIndex", s.target_id_},
      {"direction", s.direction_},
      {"state", s.state_},
      {"position", {{"x", s.position_.x_}, {"y", s.position_.y_}}},
      {"dropPoint", {{"x", s.drop_point_.x_}, {"y", s.drop_point_.y_}}},
      {"aimPoint", {{"x", s.aim_point_.x_}, {"y", s.aim_point_.y_}}},
      {"predictedTarget", {{"x", s.predicted_target_.x_}, {"y", s.predicted_target_.y_}}},
      {"targetPosition", {{"x", s.target_position_.x_}, {"y", s.target_position_.y_}}},
      {"timeSecSinceStart", s.elapsed_},
      {"currentSpeed", s.current_speed_},
      {"ammo", {{"mass", s.ammo_mass_}, {"drag", s.ammo_drag_}, {"lift", s.ammo_lift_}}},
      {"fallParameters", {{"time", s.fall_time_}, {"distance", s.fall_distance_}}},
      {"targets", targets},
    });
  }

  std::ofstream ofs("simulation.json");
  ofs << j.dump(2);
}

int main(int argc, char* argv[])
{
  if (argc < 5) {
    std::cerr << "Usage: " << argv[0] << " <gpiochip> <serial_device> <start_line> <end_line>\n";
    return 1;
  }

  const char* gpiochip_name = argv[1];
  const char* serial = argv[2];
  const unsigned int start_line = static_cast<unsigned int>(std::stoul(argv[3]));
  const unsigned int end_line = static_cast<unsigned int>(std::stoul(argv[4]));

  try {
    GpioSignal gpio(gpiochip_name);
    gpio.request_output(start_line, 0);
    gpio.request_output(end_line, 0);
    std::cout << "occupied " << gpiochip_name << " line " << start_line << ", set low\n";

    std::cout << "press y + enter to start the checker: ";
    std::string answer;
    std::getline(std::cin, answer);
    if (answer != "y") {
      std::cerr << "aborted\n";
      return 1;
    }

    gpio.set_high(start_line);
    std::cout << "set line " << start_line << " high — checker should start sending\n";

    std::cout << "waiting for ammo, config and telemetry packets...\n";
    MissionFactory factory;
    SimulationBundle sim = factory.create(LoaderType::UART, SolverType::TABLE, serial, nullptr, nullptr, BALLISTIC_TABLE_PATH);
    std::cout << "listening on " << serial << "\n";

    std::latch mission_latch{1};
    sim.mission->start(mission_latch, MAX_ITERATIONS);

    sim.mission->join();  // blocks until the drone reaches the fire point (or MAX_ITERATIONS)

    gpio.pulse_high(end_line, MISSION_DONE_PULSE);
    std::cout << "mission complete — pulsed line " << end_line << "\n";

    const std::vector<SimulationStep>& steps = sim.mission->get_steps();
    dump_json(steps);

    std::cout << "Wrote " << steps.size() << " steps to simulation.json" << std::endl;
  }
  catch (const std::exception& e) {
    std::cerr << "test harness failed: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
