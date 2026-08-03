#include "GpioSignal.hpp"
#include "JsonHttp.hpp"
#include "TcpLink.hpp"
#include "models/SimulationStep.hpp"
#include "service/MissionFactory.hpp"

#include "json.hpp"

#include <chrono>
#include <exception>
#include <format>
#include <fstream>
#include <iostream>
#include <latch>
#include <optional>
#include <string>
#include <vector>

static constexpr int MAX_ITERATIONS = 10000;
static constexpr std::chrono::milliseconds MISSION_DONE_PULSE{100};
static constexpr const char* BALLISTIC_TABLE_PATH = "homework_07/data/ballistic_table.txt";
static constexpr const char* SIMULATION_JSON_PATH = "simulation.json";
static constexpr const char* REPORT_HOST = "cppmiltech.com.ua";

static nlohmann::ordered_json build_report(const std::vector<SimulationStep>& steps,
                                           const std::string& student_id,
                                           const std::string& test_id)
{
  nlohmann::ordered_json j;
  j["studentId"] = student_id;
  j["testId"] = test_id;

  nlohmann::ordered_json& simulation = j["simulation"];
  simulation["totalSteps"] = steps.size();
  simulation["steps"] = nlohmann::json::array();

  for (const SimulationStep& s : steps) {
    nlohmann::ordered_json targets = nlohmann::json::array();
    for (const Target& t : s.all_targets_) {
      targets.push_back({
        {"id", t.target_id_},
        {"x", t.pos_.x_},
        {"y", t.pos_.y_},
      });
    }

    simulation["steps"].push_back({{"targetIndex", s.target_id_},
                                   {"direction", s.direction_},
                                   {"state", s.state_},
                                   {"position", {{"x", s.position_.x_}, {"y", s.position_.y_}}},
                                   {"dropPoint", {{"x", s.drop_point_.x_}, {"y", s.drop_point_.y_}}},
                                   {"aimPoint", {{"x", s.aim_point_.x_}, {"y", s.aim_point_.y_}}},
                                   {"predictedTarget", {{"x", s.predicted_target_.x_}, {"y", s.predicted_target_.y_}}},
                                   {"targetPosition", {{"x", s.target_position_.x_}, {"y", s.target_position_.y_}}},
                                   {"timeSecSinceStart", s.elapsed_}});
  }

  return j;
}

static void write_json_file(const std::string& path, const nlohmann::ordered_json& j)
{
  std::ofstream ofs(path);
  ofs << j.dump(2);
}

static bool post_report(const TcpLink& link,
                        const nlohmann::ordered_json& report,
                        const std::string& student_id,
                        const std::string& test_id,
                        const std::string& api_key)
{
  const HttpHeaders headers = {
    {"User-Agent",
     "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 "
     "(KHTML, like Gecko) Chrome/150.0.0.0 Safari/537.36"},
    {"Cookie", "wssplashchk=5c7ecedb1fc6640e98082a16e749284809eb2246.1785740698.1"},
    {"x-api-key", api_key},
  };

  const std::string path = std::format("/api/dz12/results/{}/{}", test_id, student_id);
  const std::optional<HttpResponse> response = post_json(link, REPORT_HOST, path, report, headers);

  if (!response) {
    std::cerr << "reporting failed: no answer from " << REPORT_HOST << path << "\n";
    return false;
  }

  std::cout << "reported to " << REPORT_HOST << path << " -> " << response->status_code << " " << response->status_text << "\n";

  return response->status_code.starts_with("2");
}

int main(int argc, char* argv[])
{
  const std::string studentId = "2063";
  const std::string taskId = "T01";

  if (argc < 6) {
    std::cerr << "Usage: " << argv[0] << " <gpiochip> <serial_device> <start_line> <end_line> <api_key>\n";
    return 1;
  }

  const char* gpiochip_name = argv[1];
  const char* serial = argv[2];
  const unsigned int start_line = static_cast<unsigned int>(std::stoul(argv[3]));
  const unsigned int end_line = static_cast<unsigned int>(std::stoul(argv[4]));
  const std::string api_key = argv[5];

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
    const nlohmann::ordered_json report = build_report(steps, studentId, taskId);

    write_json_file(SIMULATION_JSON_PATH, report);
    std::cout << "Wrote " << steps.size() << " steps to " << SIMULATION_JSON_PATH << std::endl;

    const TcpLink link;
    if (!post_report(link, report, studentId, taskId, api_key)) {
      return 1;
    }
  }
  catch (const std::exception& e) {
    std::cerr << "test harness failed: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
