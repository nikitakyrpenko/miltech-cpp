#include "ballistic.hpp"

#include <cmath>

float calcTAmmo(const Drone &drone, const Ammo &ammo)
{
  float a = (ammo.drag * G * ammo.mass) - (2.0f * ammo.drag * ammo.drag * ammo.lift * drone.at);
  float b = (-3.0f * G * ammo.mass * ammo.mass) + (3.0f * ammo.drag * ammo.lift * ammo.mass * drone.at);
  float c = 6.0f * ammo.mass * ammo.mass * drone.position.z;

  float p = (-1.0f * b * b) / (3.0f * a * a);
  float q = (2.0f * b * b * b) / (27.0f * a * a * a) + (c / a);

  float acos_arg = (3.0f * q) / (2.0f * p) * std::sqrt(-3.0f / p);

  if (acos_arg < -1.0f || acos_arg > 1.0f) {
    return 0.0;
  };

  float ttf = 2.0f * std::sqrt(-p / 3.0f) * std::cos((std::acos(acos_arg) + 4.0f * std::acos(-1.0f)) / 3.0f) - b / (3.0f * a);
  return ttf;
}

float calcHDist(const Drone &drone, const Ammo &ammo, float tAmmo)
{
  float d = ammo.drag;
  float l = ammo.lift;
  float m = ammo.mass;
  float attackSpeed = drone.at;
  float dtf = (std::pow(tAmmo, 3) * ((6.0f * d * G * l * m) - (6.0f * std::pow(d, 2) * (std::pow(l, 2) - 1) * attackSpeed))) /
                (36.0f * std::pow(m, 2)) +
              (std::pow(tAmmo, 5) * ((3.0f * std::pow(d, 3) * G * std::pow(l, 3) * m) -
                                     (3.0f * std::pow(d, 4) * std::pow(l, 2) * (std::pow(l, 2) + 1) * attackSpeed))) /
                (36.0f * (std::pow(l, 2) + 1) * std::pow(m, 4)) +
              (std::pow(tAmmo, 4) * ((3.0f * std::pow(d, 3) * (std::pow(l, 2) + 1) * std::pow(l, 2) * attackSpeed) +
                                     (6.0f * std::pow(d, 3) * (std::pow(l, 2) + 1) * std::pow(l, 4) * attackSpeed) -
                                     (6.0f * std::pow(d, 2) * G * (std::pow(l, 4) + std::pow(l, 2) + 1) * l * m))) /
                (36.0f * std::pow(std::pow(l, 2) + 1, 2) * std::pow(m, 3)) -
              (d * std::pow(tAmmo, 2) * attackSpeed) / (2.0f * m) + (tAmmo * attackSpeed);
  return dtf;
}

Result calcFirePosition(const Drone &drone, const Ammo &ammo, const Coord &target, FirePosition &outFirePosition)
{
  float ttf = calcTAmmo(drone, ammo);
  if (ttf == 0.0) {
    return Result::DroneToHigh;
  }

  float dtf = calcHDist(drone, ammo, ttf);

  float D = std::sqrt(std::pow(target.x - drone.position.x, 2) + std::pow(target.y - drone.position.y, 2));

  if (dtf + drone.ap < D) {
    float ratio = (D - dtf) / D;

    float fireX = drone.position.x + (target.x - drone.position.x) * ratio;
    float fireY = drone.position.y + (target.y - drone.position.y) * ratio;

    Coord fire{fireX, fireY, drone.position.z};

    outFirePosition.intermidiate = fire;
    outFirePosition.fire = fire;
    outFirePosition.hasIntermidiate = false;

    return Result::OK;
  }
  else {
    float intermidiateX, intermidiateY;
    // distance between target and drone is zero -> increment to +X direction
    if (D == 0) {
      intermidiateX = target.x + (dtf + drone.ap);
      intermidiateY = target.y;
    }
    else {
      intermidiateX = target.x - (target.x - drone.position.x) * (dtf + drone.ap) / D;
      intermidiateY = target.y - (target.y - drone.position.y) * (dtf + drone.ap) / D;
    }

    float D = std::sqrt(std::pow(target.x - intermidiateX, 2) + std::pow(target.y - intermidiateY, 2));
    float ratio = (D - dtf) / D;

    float fireX = intermidiateX + (target.x - intermidiateX) * ratio;
    float fireY = intermidiateY + (target.y - intermidiateY) * ratio;

    outFirePosition.intermidiate = {intermidiateX, intermidiateY, drone.position.z};
    outFirePosition.fire = {fireX, fireY, drone.position.z};
    outFirePosition.hasIntermidiate = true;

    return Result::OK;
  }
}