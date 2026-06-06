#include "parser.hpp"

#include <cstring>
#include <istream>
#include "dto.hpp"

static constexpr float kVog17Mass = 0.35F;
static constexpr float kVog17Drag = 0.07F;

static constexpr float kM67Mass = 0.60F;
static constexpr float kM67Drag = 0.10F;

static constexpr float kRkg3Mass = 1.20F;
static constexpr float kRkg3Drag = 0.10F;

static constexpr float kGlidingVogMass = 0.45F;
static constexpr float kGlidingVogDrag = 0.10F;
static constexpr float kGlidingVogLift = 1.0F;

static constexpr float kGlidingRkgMass = 1.40F;
static constexpr float kGlidingRkgDrag = 0.10F;
static constexpr float kGlidingRkgLift = 1.0F;

ParsingResult parse(std::istream& stream, Drone& drone, Ammo& ammo, Coord& target)
{
  float drone_x{-1.0F};
  float drone_y{-1.0F};
  float drone_z{-1.0F};
  float target_x{-1.0F};
  float target_y{-1.0F};
  float attack_speed{-1.0F};
  float acceleration_path{-1.0F};

  char ammo_type[maxAmmoSize]{};
  float mass{-1.0F};
  float drag{-1.0F};
  float lift{-1.0F};

  if (!(stream >> drone_x >> drone_y >> drone_z >> target_x >> target_y >> attack_speed >> acceleration_path >> ammo_type)) {
    return ParsingResult::Malformed;
  }

  // NOLINTBEGIN(cppcoreguidelines-pro-bounds-array-to-pointer-decay): strcmp requires char*, decay unavoidable
  if (std::strcmp(ammo_type, "VOG-17") == 0) {
    mass = kVog17Mass, drag = kVog17Drag, lift = 0.0F;
  }
  else if (std::strcmp(ammo_type, "M67") == 0) {
    mass = kM67Mass, drag = kM67Drag, lift = 0.0F;
  }
  else if (std::strcmp(ammo_type, "RKG-3") == 0) {
    mass = kRkg3Mass, drag = kRkg3Drag, lift = 0.0F;
  }
  else if (std::strcmp(ammo_type, "GLIDING-VOG") == 0) {
    mass = kGlidingVogMass, drag = kGlidingVogDrag, lift = kGlidingVogLift;
  }
  else if (std::strcmp(ammo_type, "GLIDING-RKG") == 0) {
    mass = kGlidingRkgMass, drag = kGlidingRkgDrag, lift = kGlidingRkgLift;
  }
  else {
    return ParsingResult::UnknownAmmo;
  }
  // NOLINTEND(cppcoreguidelines-pro-bounds-array-to-pointer-decay)

  if (drone_z <= 0.0F) {
    return ParsingResult::AltitudeOutOfRange;
  }
  if (acceleration_path <= 0.0F) {
    return ParsingResult::AccelerationPathOutOfRange;
  }
  if (attack_speed <= 0.0F) {
    return ParsingResult::AttackSpeedOutOfRange;
  }

  drone.position_ = {drone_x, drone_y, drone_z};
  drone.ap_ = acceleration_path;
  drone.at_ = attack_speed;

  target.x_ = target_x;
  target.y_ = target_y;
  target.z_ = 0.0F;

  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay) strncpy requires char*, decay unavoidable
  std::strncpy(ammo.name_, ammo_type, maxAmmoSize);
  ammo.mass_ = mass;
  ammo.drag_ = drag;
  ammo.lift_ = lift;

  return ParsingResult::OK;
}