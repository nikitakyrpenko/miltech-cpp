#pragma once

#include <cstdint>

constexpr std::uint8_t maxAmmoSize = 32;

enum class ParsingResult : std::uint8_t {
  OK,
  Malformed,
  UnknownAmmo,
  AttackSpeedOutOfRange,
  AccelerationPathOutOfRange,
  AltitudeOutOfRange
};
enum class ComputationResult : std::uint8_t { OK, AltitudeExceeded };

struct Coord {
  float x_{};
  float y_{};
  float z_{};
};

struct Drone {
  Coord position_{};
  float at_{};
  float ap_{};
};

struct Ammo {
  char name_[maxAmmoSize]{};
  float mass_{};
  float drag_{};
  float lift_{};
};

struct FirePosition {
  Coord intermidiate_{};
  Coord fire_{};
  bool has_intermidiate_{};
};