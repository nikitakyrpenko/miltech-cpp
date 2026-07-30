#include "c2_controller.hpp"
#include "fc_link.hpp"     // MAVSDK обгортка, API описано у fc_link.hpp
#include "udp_socket.hpp"  // UDP прийом, API описано у udp_socket.hpp
#include "ned_position.hpp"

#include <cstddef>
#include <cstdint>
#include <nlohmann/json.hpp>  // Розбiр JSON з точками маршруту вiд auto_stub

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>

static constexpr uint16_t STUB_PORT = 14560;
static constexpr size_t BUFFER_SIZE = 2048;

static constexpr const char* LOG_PATH = "/var/log/c2/c2.log";

static std::string log_time()
{
  using namespace std::chrono;

  const auto now = system_clock::now();
  const auto t = system_clock::to_time_t(now);
  const auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

  std::tm tm{};
  localtime_r(&t, &tm);

  std::ostringstream os;
  os << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << '.' << std::setfill('0') << std::setw(3) << ms.count();
  return os.str();
}

struct C2Controller::Impl {
  C2State state;
  std::optional<NEDPosition> position;

  UdpSocket sock;
  FcLink fc;

  std::ofstream logs;
  bool ready{false};

  Impl(uint16_t fc_port)
    : state(C2State::DISARMED)
    , sock(STUB_PORT)
    , fc(fc_port)
    , logs(LOG_PATH, std::ios::app)
  {
    if (!logs.is_open()) {
      std::cout << "[" << log_time() << "] [C2] warning: cannot open " << LOG_PATH << " — logging to stdout only" << std::endl;
    }
  }

  template <typename... Args>
  void log(Args&&... args)
  {
    std::ostringstream m;
    (m << ... << std::forward<Args>(args));
    const std::string line = "[" + log_time() + "] " + m.str();
    std::cout << line << std::endl;
    if (logs.is_open()) {
      logs << line << '\n';
      logs.flush();
    }
  }

  void poll_health()
  {
    if (!ready && fc.is_connected()) {
      std::ofstream("/tmp/c2_healthy").close();
      ready = true;
    }
  }

  bool poll_position()
  {
    uint8_t b[BUFFER_SIZE]{};
    const auto c = sock.recv(b, BUFFER_SIZE);

    if (c < 0) {
      return false;
    }

    float north_m, east_m;

    try {
      const auto j = nlohmann::json::parse(b, b + c);

      north_m = j.at("north_m").get<float>();
      east_m = j.at("east_m").get<float>();
    }
    catch (const nlohmann::json::exception& e) {
      log("[C2] error: cannot parse incoming position : ", e.what());
      return false;
    }

    NEDPosition pos{.north_m = north_m, .east_m = east_m};

    if (position == pos) {
      return false;
    }

    position = std::move(pos);

    log("[C2] recv: north=", north_m, " east=", east_m);

    return true;
  }

  std::optional<C2State> get_state()
  {
    if (!fc.is_armed()) {
      return C2State::DISARMED;
    }
    switch (fc.flight_mode()) {
      case FcLink::FlightMode::Guided: {
        return C2State::ARMED_GUIDED;
      }
      case FcLink::FlightMode::Hold: {
        return C2State::ARMED_HOLD;
      }
      case FcLink::FlightMode::Manual: {
        return C2State::ARMED_MANUAL;
      }
      default: {
        return std::nullopt;
      }
    }
  }

  void on_transition(std::optional<C2State> next)
  {
    if (state == next) {
      return;
    }

    log("[C2] state: ", to_string(state), " -> ", to_string(*next));

    if ((state == C2State::ARMED_MANUAL || state == C2State::ARMED_GUIDED) && next == C2State::ARMED_HOLD) {
      fc.hold();
      log("[C2] blocked: waypoint in STATE: ", to_string(*next));
    }

    state = *next;
  }

  void handle_state()
  {
    if (state == C2State::ARMED_GUIDED && position) {
      fc.go_to_ned(position->north_m, position->east_m);
      return;
    }
  }
};

C2Controller::C2Controller(uint16_t fc_port)
  : impl_(std::make_unique<Impl>(fc_port))
{
}

C2Controller::~C2Controller() = default;

void C2Controller::tick()
{
  impl_->poll_health();
  impl_->poll_position();
  const auto state = impl_->get_state();

  if (state) {
    impl_->on_transition(state);
  }

  impl_->handle_state();
}

C2State C2Controller::current_state() const
{
  return impl_->state;
}
