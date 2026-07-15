#pragma once

#include <cstdint>

namespace antidrone_turret {

enum class Decision { IDLE, TRACK, REQUEST_SHOT };
enum class ServoCommand : int8_t { RIGHT = 1, LEFT = -1, CENTER = 0 };
enum class GimbalCommand : int8_t { UP = 1, DOWN = -1, CENTER = 0 };
enum class Axis { X, Y };

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

inline Decision decide(const TargetInput& target, const DecisionConfig& config)
{
  if (!target.visible) {
    return Decision::IDLE;
  }

  if (target.confidence < config.confidence_threshold) {
    return Decision::IDLE;
  }

  if (target.distance_m <= config.max_distance_m) {
    return Decision::REQUEST_SHOT;
  }

  return Decision::TRACK;
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