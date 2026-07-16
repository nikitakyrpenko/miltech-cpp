#pragma once

#include <cstdint>

namespace antidrone_turret {

enum class ServoCommand : int8_t { RIGHT = 1, LEFT = -1, CENTER = 0 };
enum class GimbalCommand : int8_t { UP = 1, DOWN = -1, CENTER = 0 };

enum class Confidence : int8_t { NONE = 0, LOW = 1, HIGH = 2 };
enum class Lock : int8_t { IDLE = 0, TRACK = 1 };
enum class Trigger : int8_t { SKIP = 0, REQUESTED = 1, RELOADING = 2 };

enum class ActuatorState : int8_t { READY = 0, RELOADING = 1 };

// config yaml sourced
struct DecisionConfig {
  float confidence_threshold{0.8F};
  float max_distance_m{30.0F};
};

// config yaml sourced
struct ScreenResolutionConfig {
  float width{640.0F};
  float height{480.0F};
};

struct TargetInput {
  bool visible;
  float x;
  float y;
  float distance_m;
  float confidence;
};

struct ServoParameters {
  ServoCommand command;
  float target_x;
  float error_x;
};

struct GimbalParameters {
  GimbalCommand command;
  float target_y;
  float error_y;
};

struct State {
  Confidence confidence;
  Lock lock;
  Trigger trigger;
};

struct ActuatorStatus {
  ActuatorState state;
  int count;
};

inline State decide(const TargetInput& target, const DecisionConfig& config, const ActuatorStatus& actuator_status)
{
  if (!target.visible) {
    return {Confidence::NONE, Lock::IDLE, Trigger::SKIP};
  }
  if (target.confidence < config.confidence_threshold) {
    return {Confidence::LOW, Lock::IDLE, Trigger::SKIP};
  }
  if (actuator_status.state == ActuatorState::RELOADING) {
    return {Confidence::HIGH, Lock::TRACK, Trigger::RELOADING};
  }
  if (target.distance_m <= config.max_distance_m) {
    return {Confidence::HIGH, Lock::TRACK, Trigger::REQUESTED};
  }
  return {Confidence::HIGH, Lock::TRACK, Trigger::SKIP};
}

inline ServoParameters calculate_servo_parameters(const ScreenResolutionConfig& resolution, float x_coord)
{
  float error_x = resolution.width / 2 - x_coord;
  ServoCommand command{};

  if (error_x > 0) {
    command = ServoCommand::RIGHT;
  }
  else if (error_x < 0) {
    command = ServoCommand::LEFT;
  }
  else {
    command = ServoCommand::CENTER;
  }
  return ServoParameters{command, x_coord, error_x};
}

inline GimbalParameters calculate_gimbal_parameters(const ScreenResolutionConfig& resolution, float y_coord)
{
  float error_y = resolution.height / 2 - y_coord;
  GimbalCommand command{};

  if (error_y > 0) {
    command = GimbalCommand::UP;
  }
  else if (error_y < 0) {
    command = GimbalCommand::DOWN;
  }
  else {
    command = GimbalCommand::CENTER;
  }
  return GimbalParameters{command, y_coord, error_y};
}
}  // namespace antidrone_turret