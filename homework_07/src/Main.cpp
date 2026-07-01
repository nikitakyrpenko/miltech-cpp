#include "GpioSignal.hpp"
#include "ThreadSafeQueue.hpp"
#include "UartConfigLoader.hpp"
#include "UartDronePhysics.hpp"
#include "UartLink.hpp"
#include "UartTargetProvider.hpp"
#include "models/SimulationStep.hpp"
#include "service/AnalyticalBallisticSolver.hpp"
#include "service/BallisticSolutionEvaluator.hpp"
#include "service/ConfigLoader.hpp"
#include "service/FirepointProvider.hpp"
#include "service/MissionProccesor.hpp"

#include "json.hpp"

#include <chrono>
#include <exception>
#include <fstream>
#include <iostream>
#include <latch>
#include <string>
#include <thread>
#include <vector>

static constexpr int MAX_ITERATIONS = 10000;
static constexpr unsigned int MISSION_DONE_LINE = 100;
static constexpr std::chrono::milliseconds MISSION_DONE_PULSE{100};

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
  if (argc < 7) {
    std::cerr << "Usage: " << argv[0] << " <gpiochip> <start_line> <serial_device> <config.json> <ammo.json> <targets.json>\n";
    return 1;
  }

  const char* gpiochip_name = argv[1];
  const unsigned int start_line = static_cast<unsigned int>(std::stoul(argv[2]));
  const char* serial = argv[3];

  try {
    ConfigLoader config_loader(argv[4], argv[5], argv[6]);
    const auto& cfg = config_loader.get_config();

    auto link = std::make_shared<UartLink>(serial);
    auto target_provider = std::make_shared<UartTargetProvider>(link, cfg.target_timestep);

    std::latch latch{2};
    link->start(latch);
    target_provider->start(latch);
    std::cout << "listening on " << serial << "\n";

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

    std::cout << "waiting for ammo packet...\n";
    auto uart_config = std::make_shared<UartConfigLoader>(link, cfg);
    std::cout << "ammo received: " << uart_config->get_config().ammo_ << "\n";

    auto command_channel = std::make_shared<SynchronizedQueue<DroneCommand>>();
    auto drone_physics = std::make_shared<UartDronePhysics>(link, *uart_config, command_channel, cfg.physics_timestep);

    std::latch physics_latch{1};
    drone_physics->start(physics_latch);

    std::cout << "waiting for first telemetry frame...\n";
    while (!drone_physics->is_ready()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    auto mission = std::make_unique<MissionProccessor>(target_provider,
                                                        drone_physics,
                                                        uart_config,
                                                        std::make_unique<AnalyticalBallisticSolver>(),
                                                        std::make_unique<FirepointProvider>(),
                                                        std::make_unique<BallisticSolutionEvaluator>(),
                                                        command_channel);

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
