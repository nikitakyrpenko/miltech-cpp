#include "FileResultReporter.hpp"
#include "GpioSignal.hpp"
#include "HttpResultReporter.hpp"
#include "models/SimulationStep.hpp"
#include "service/MissionFactory.hpp"

#include <chrono>
#include <exception>
#include <iostream>
#include <latch>
#include <string>
#include <vector>

static constexpr int MAX_ITERATIONS = 10000;
static constexpr std::chrono::milliseconds MISSION_DONE_PULSE{100};
static constexpr const char* BALLISTIC_TABLE_PATH = "homework_07/data/ballistic_table.txt";

static constexpr const char* REPORT_HOST = "cppmiltech.com.ua";
static constexpr const char* REPORT_PATH = "/api/dz12/results";
static constexpr std::string CURRENT_STUDENT_ID = "2063";

int main(int argc, char* argv[])
{
  if (argc < 7) {
    std::cerr << "Usage: " << argv[0] << " <gpiochip> <serial_device> <start_line> <end_line> <api_key> <task_id>\n";
    return 1;
  }

  const char* gpiochip_name = argv[1];
  const char* serial = argv[2];
  const unsigned int start_line = static_cast<unsigned int>(std::stoul(argv[3]));
  const unsigned int end_line = static_cast<unsigned int>(std::stoul(argv[4]));
  const std::string API_KEY = argv[5];
  const std::string TASK_ID = argv[6];

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

    const std::vector<SimulationStep>& steps = sim.mission->get_steps();
    std::cout << "mission complete with steps:" << steps.size() << "— pulsed line :" << end_line << "\n";

    HttpResultReporter http_reporter(REPORT_HOST, REPORT_PATH, API_KEY);
    // FileResultReporter http_reporter("simulation.json");

    if (!http_reporter.save(steps, CURRENT_STUDENT_ID, TASK_ID)) {
      std::cerr << "report failed to save\n";
      return 1;
    }

    if (!http_reporter.check(CURRENT_STUDENT_ID, TASK_ID)) {
      std::cerr << "report saved but GET-check did not confirm it on the server\n";
      return 1;
    }
  }
  catch (const std::exception& e) {
    std::cerr << "test harness failed: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
