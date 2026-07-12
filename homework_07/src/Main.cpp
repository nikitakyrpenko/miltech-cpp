#include "GpioSignal.hpp"
#include "UartConfigLoader.hpp"
#include "UartDronePhysics.hpp"
#include "UartLink.hpp"
#include "UartTargetProvider.hpp"
#include "models/SimulationStep.hpp"
#include "service/BallisticSolutionEvaluator.hpp"
#include "service/FirepointProvider.hpp"
#include "service/MissionProccesor.hpp"

#include "json.hpp"
#include "service/BallisticTableLoader.hpp"
#include "service/TableBallisticSolver.hpp"

#include <chrono>
#include <exception>
#include <fstream>
#include <iostream>
#include <latch>
#include <memory>
#include <string>
#include <vector>

static constexpr int MAX_ITERATIONS = 10000;
static constexpr unsigned int MISSION_DONE_LINE = 100;
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
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0] << " <gpiochip> <start_line> <serial_device>\n";
    return 1;
  }

  const char* gpiochip_name = argv[1];
  const unsigned int start_line = static_cast<unsigned int>(std::stoul(argv[2]));
  const char* serial = argv[3];

  try {
    std::latch la{1};
    auto link = std::make_shared<UartLink>(serial);
    link->start(la);

    auto table = load_ballistic_table(BALLISTIC_TABLE_PATH);
    if (!table) {
      std::cerr << "failed to parse ballistic table from " << BALLISTIC_TABLE_PATH << "\n";
      return 1;
    }

    GpioSignal gpio(gpiochip_name);
    gpio.request_output(start_line, 0);
    gpio.request_output(MISSION_DONE_LINE, 0);
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
    std::latch config_start{1};
    auto uart_config = std::make_shared<UartConfigLoader>(link);
    uart_config->start(config_start);
    uart_config->wait_ready();
    uart_config->set_table(std::move(*table));
    std::cout << "ammo received: " << uart_config->get_config().ammo_ << "\n";
    auto abs = std::make_unique<TableBallisticSolver>(uart_config);
    auto target_provider = std::make_shared<UartTargetProvider>(link, uart_config);

    std::latch latch{2};
    target_provider->start(latch);
    std::cout << "listening on " << serial << "\n";

    auto drone_physics = std::make_shared<UartDronePhysics>(link, uart_config);

    drone_physics->start(latch);

    auto mission = std::make_unique<MissionProccessor>(target_provider,
                                                       drone_physics,
                                                       uart_config,
                                                       std::move(abs),
                                                       std::make_unique<FirepointProvider>(),
                                                       std::make_unique<BallisticSolutionEvaluator>());

    std::latch mission_latch{1};
    mission->start(mission_latch, MAX_ITERATIONS);

    mission->join();  // blocks until the drone reaches the fire point (or MAX_ITERATIONS)

    gpio.pulse_high(MISSION_DONE_LINE, MISSION_DONE_PULSE);
    std::cout << "mission complete — pulsed line " << MISSION_DONE_LINE << "\n";

    link->interrupt();
    target_provider->interrupt();
    drone_physics->interrupt();

    const std::vector<SimulationStep>& steps = mission->get_steps();
    dump_json(steps);

    std::cout << "Wrote " << steps.size() << " steps to simulation.json" << std::endl;
  }
  catch (const std::exception& e) {
    std::cerr << "test harness failed: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
