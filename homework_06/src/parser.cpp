#include "parser.hpp"

#include <cstring>
#include <istream>
#include "dto.hpp"

ParsingResult parse(std::istream& stream, Drone& drone, Ammo& ammo, Coord& target)
{
  float xd, yd, zd, targetX, targetY, at, ap;
  char ammoType[32];
  float m, d, l;

  if (!(stream >> xd >> yd >> zd >> targetX >> targetY >> at >> ap >> ammoType)) {
    return ParsingResult::Malformed;
  }

  if (std::strcmp(ammoType, "VOG-17") == 0) {
    m = 0.35f, d = 0.07f, l = 0.0f;
  }
  else if (std::strcmp(ammoType, "M67") == 0) {
    m = 0.6f, d = 0.10f, l = 0.0f;
  }
  else if (std::strcmp(ammoType, "RKG-3") == 0) {
    m = 1.2f, d = 0.10f, l = 0.0f;
  }
  else if (std::strcmp(ammoType, "GLIDING-VOG") == 0) {
    m = 0.45f, d = 0.10f, l = 1.0f;
  }
  else if (std::strcmp(ammoType, "GLIDING-RKG") == 0) {
    m = 1.40f, d = 0.10f, l = 1.0f;
  }
  else {
    return ParsingResult::UnknownAmmo;
  }

  if (zd <= 0) {
    return ParsingResult::AltitudeOutOfRange;
  }
  if (ap <= 0) {
    return ParsingResult::AccelerationPathOutOfRange;
  }
  if (at <= 0) {
    return ParsingResult::AttackSpeedOutOfRange;
  }

  drone.position = {xd, yd, zd};
  drone.ap = ap;
  drone.at = at;

  target.x = targetX;
  target.y = targetY;
  target.z = 0;

  std::strncpy(ammo.name, ammoType, 32);
  ammo.mass = m;
  ammo.drag = d;
  ammo.lift = l;

  return ParsingResult::OK;
}