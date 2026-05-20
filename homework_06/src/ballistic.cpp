#include "ballistic.hpp"

#include <cmath>
#include "dto.hpp"

static constexpr float kG = 9.81F;

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers): Cardano's cubic formula coefficients
float calcTAmmo(const Drone &drone, const Ammo &ammo)
{
  float a = (ammo.drag_ * kG * ammo.mass_) - (2.0F * ammo.drag_ * ammo.drag_ * ammo.lift_ * drone.at_);
  float b = (-3.0F * kG * ammo.mass_ * ammo.mass_) + (3.0F * ammo.drag_ * ammo.lift_ * ammo.mass_ * drone.at_);
  float c = 6.0F * ammo.mass_ * ammo.mass_ * drone.position_.z_;

  float p = (-1.0F * b * b) / (3.0F * a * a);
  float q = (2.0F * b * b * b) / (27.0F * a * a * a) + (c / a);

  float acos_arg = (3.0F * q) / (2.0F * p) * sqrtf(-3.0F / p);

  if (acos_arg < -1.0F || acos_arg > 1.0F) {
    return 0.0F;
  };

  float ttf = 2.0F * sqrtf(-p / 3.0F) * cosf((acosf(acos_arg) + 4.0F * acosf(-1.0F)) / 3.0F) - b / (3.0F * a);
  return ttf;
}

float calcHDist(const Drone &drone, const Ammo &ammo, float tAmmo)
{
  float d = ammo.drag_;
  float l = ammo.lift_;
  float m = ammo.mass_;
  float attackSpeed = drone.at_;
  float dtf =
    (powf(tAmmo, 3) * ((6.0F * d * kG * l * m) - (6.0F * powf(d, 2) * (powf(l, 2) - 1) * attackSpeed))) / (36.0F * powf(m, 2)) +
    (powf(tAmmo, 5) * ((3.0F * powf(d, 3) * kG * powf(l, 3) * m) - (3.0F * powf(d, 4) * powf(l, 2) * (powf(l, 2) + 1) * attackSpeed))) /
      (36.0F * (powf(l, 2) + 1) * powf(m, 4)) +
    (powf(tAmmo, 4) * ((3.0F * powf(d, 3) * (powf(l, 2) + 1) * powf(l, 2) * attackSpeed) +
                       (6.0F * powf(d, 3) * (powf(l, 2) + 1) * powf(l, 4) * attackSpeed) -
                       (6.0F * powf(d, 2) * kG * (powf(l, 4) + powf(l, 2) + 1) * l * m))) /
      (36.0F * powf(powf(l, 2) + 1, 2) * powf(m, 3)) -
    (d * powf(tAmmo, 2) * attackSpeed) / (2.0F * m) + (tAmmo * attackSpeed);
  return dtf;
}
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

ComputationResult calc_fire_position(const Drone &drone, const Ammo &ammo, const Coord &target, FirePosition &out_fire_position)
{
  float ttf = calcTAmmo(drone, ammo);
  if (ttf == 0.0F) {
    return ComputationResult::AltitudeExceeded;
  }

  float dtf = calcHDist(drone, ammo, ttf);

  float D = sqrtf(powf(target.x_ - drone.position_.x_, 2) + powf(target.y_ - drone.position_.y_, 2));

  if (dtf + drone.ap_ < D) {
    float ratio = (D - dtf) / D;

    float fireX = drone.position_.x_ + (target.x_ - drone.position_.x_) * ratio;
    float fireY = drone.position_.y_ + (target.y_ - drone.position_.y_) * ratio;

    Coord fire{fireX, fireY, drone.position_.z_};

    out_fire_position.intermidiate_ = fire;
    out_fire_position.fire_ = fire;
    out_fire_position.has_intermidiate_ = false;

    return ComputationResult::OK;
  }

  float intermidiateX{};
  float intermidiateY{};
  // distance between target and drone is zero -> increment to +X direction
  if (D == 0) {
    intermidiateX = target.x_ + (dtf + drone.ap_);
    intermidiateY = target.y_;
  }
  else {
    intermidiateX = target.x_ - (target.x_ - drone.position_.x_) * (dtf + drone.ap_) / D;
    intermidiateY = target.y_ - (target.y_ - drone.position_.y_) * (dtf + drone.ap_) / D;
  }

  float D2 = sqrtf(powf(target.x_ - intermidiateX, 2) + powf(target.y_ - intermidiateY, 2));
  float ratio = (D2 - dtf) / D2;

  float fireX = intermidiateX + (target.x_ - intermidiateX) * ratio;
  float fireY = intermidiateY + (target.y_ - intermidiateY) * ratio;

  out_fire_position.intermidiate_ = {intermidiateX, intermidiateY, drone.position_.z_};
  out_fire_position.fire_ = {fireX, fireY, drone.position_.z_};
  out_fire_position.has_intermidiate_ = true;

  return ComputationResult::OK;
}
