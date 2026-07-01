#include "UartTargetProvider.hpp"
#include "models/SimulationStep.hpp"
#include "service/MissionFactory.hpp"

#include "json.hpp"

#include <exception>
#include <fstream>
#include <iostream>
#include <latch>
#include <vector>
/*
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
*/

#include "DroneLink.hpp"
#include "GpioSignal.hpp"
#include "ThreadSafeQueue.hpp"
#include "UartReader.hpp"

#include <chrono>
#include <iostream>
#include <latch>
#include <string>
#include <thread>

int main(int argc, char* argv[])
{
  if (argc < 4) {
    std::cerr << "Usage: " << argv[0] << " <gpiochip> <start_line> <serial_device>\n";
    return 1;
  }

  const char* gpiochip_name = argv[1];
  const unsigned int start_line = static_cast<unsigned int>(std::stoul(argv[2]));
  const char* serial = argv[3];

  SynchronizedQueue<dlink::Telemetry> telemetry_q;
  SynchronizedQueue<dlink::TargetPos> target_q;
  SynchronizedQueue<dlink::AmmoCfg> ammo_q;
  SynchronizedQueue<dlink::Result> result_q;
  SynchronizedQueue<dlink::DroneCfg> cfg_q;

  try {
    UartReader reader(serial, telemetry_q, target_q, ammo_q, result_q, cfg_q);
    UartTargetProvider target_provider(target_q, 0.1F);

    std::latch latch{2};
    reader.run(latch);
    target_provider.start(latch);
    std::cout << "listening on " << serial << "\n";

    GpioSignal gpio(gpiochip_name);
    gpio.request_output(start_line, 0);
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

    while (true) {
      if (auto t = telemetry_q.drain_to_last()) {
        std::cout << "TELEMETRY t_ms=" << t->t_ms << " x=" << t->x << " y=" << t->y << " z=" << t->z << " vx=" << t->vx << " vy=" << t->vy
                  << " speed=" << t->speed << " dir=" << t->dir << " state=" << static_cast<int>(t->state) << "\n";
      }
      if (auto a = ammo_q.drain_to_last()) {
        std::cout << "AMMO name=" << a->name << " mass=" << a->mass << " drag=" << a->drag << " lift=" << a->lift
                  << " hitRadius=" << a->hitRadius << " nTargets=" << static_cast<int>(a->nTargets) << "\n";
      }
      for (const auto& tg : target_provider.get_targets()) {
        std::cout << "TARGET id=" << tg.target_id_ << " x=" << tg.pos_.x_ << " y=" << tg.pos_.y_ << "\n";
      }
      if (auto r = result_q.drain_to_last()) {
        std::cout << "RESULT hit=" << static_cast<int>(r->hit) << " targetId=" << static_cast<int>(r->targetId) << " miss_m=" << r->miss_m
                  << " drop_t_ms=" << r->drop_t_ms << "\n";
      }
      if (auto c = cfg_q.drain_to_last()) {
        std::cout << "CONFIG attackSpeed=" << c->attackSpeed << " accelerationPath=" << c->accelerationPath
                  << " angularSpeed=" << c->angularSpeed << " turnThreshold=" << c->turnThreshold << " timeStep=" << c->timeStep
                  << " timeScale=" << c->timeScale << "\n";
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }
  catch (const std::exception& e) {
    std::cerr << "test harness failed: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
