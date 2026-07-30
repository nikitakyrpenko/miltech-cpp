#pragma once
#include <cstdint>
#include <memory>
#include <string>

enum class C2State {
  DISARMED,
  ARMED_HOLD,
  ARMED_GUIDED,
  ARMED_MANUAL,
};

inline std::string to_string(C2State s)
{
  switch (s) {
    case C2State::DISARMED:
      return "DISARMED";
    case C2State::ARMED_HOLD:
      return "ARMED_HOLD";
    case C2State::ARMED_GUIDED:
      return "ARMED_GUIDED";
    case C2State::ARMED_MANUAL:
      return "ARMED_MANUAL";
  }
  return "UNKNOWN";
}

// C2-сервiс для арбiтражу команд.
// Читає MAVLink-телеметрiю з fc_sim через MAVSDK та точки маршруту
// вiд auto_stub у JSON через UDP. Вирiшує, хто може командувати FC.
class C2Controller {
public:
  explicit C2Controller(uint16_t fc_port);
  ~C2Controller();

  // Обробити одну iтерацiю: передати або блокувати команди
  // залежно вiд поточного стану. Викликати з main loop приблизно на 10 Hz.
  void tick();

  // Read-only доступ до поточного стану C2.
  // Корисно для перевiрок, тестiв або майбутньої дiагностики.
  C2State current_state() const;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};
