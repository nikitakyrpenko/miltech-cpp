#include <gtest/gtest.h>

#include "ballistic.hpp"
#include "dto.hpp"
TEST(DroneBallisticCalc, noIntermidiatePosition_ammoVOG_17)
{
  const Drone drone{.position_{180.F, 180.F, 100.F}, .at_ = 10.F, .ap_ = 10.F};
  const Ammo ammo{.name_ = "VOG-17", .mass_ = 0.35F, .drag_ = 0.07F, .lift_ = 0.0F};
  const Coord target{200.F, 200.F, 0.F};

  FirePosition res{};

  ComputationResult status = calc_fire_position(drone, ammo, target, res);

  EXPECT_NEAR(res.fire_.x_, 173.759F, 0.01F);
  EXPECT_NEAR(res.fire_.y_, 173.759F, 0.01F);
  EXPECT_EQ(res.fire_.z_, drone.position_.z_);

  EXPECT_NEAR(res.intermidiate_.x_, 166.688F, 0.01F);
  EXPECT_NEAR(res.intermidiate_.y_, 166.688F, 0.01F);
  EXPECT_EQ(res.intermidiate_.z_, drone.position_.z_);

  EXPECT_EQ(res.has_intermidiate_, true);
  EXPECT_EQ(status, ComputationResult::OK);
}

TEST(DroneBallisticCalc, noIntermidiatePosition_ammoGLIDING_VOG)
{
  const Drone drone{.position_{0.F, 0.F, 100.F}, .at_ = 20.F, .ap_ = 50.F};
  const Ammo ammo{.name_ = "GLIDING-VOG", .mass_ = 0.45F, .drag_ = 0.10F, .lift_ = 1.0F};
  const Coord target{300.F, 300.F, 0.F};

  FirePosition res{};

  ComputationResult status = calc_fire_position(drone, ammo, target, res);

  EXPECT_NEAR(res.fire_.x_, 242.711F, 0.01F);
  EXPECT_NEAR(res.fire_.y_, 242.711F, 0.01F);
  EXPECT_EQ(res.fire_.z_, drone.position_.z_);

  EXPECT_NEAR(res.intermidiate_.x_, 242.711F, 0.01F);
  EXPECT_NEAR(res.intermidiate_.y_, 242.711F, 0.01F);
  EXPECT_EQ(res.intermidiate_.z_, drone.position_.z_);

  EXPECT_EQ(res.has_intermidiate_, false);
  EXPECT_EQ(status, ComputationResult::OK);
}

TEST(DroneBallisticCalc, noIntermidiatePosition_ammoGLIDING_RKG)
{
  const Drone drone{.position_{543.F, 232.F, 120.F}, .at_ = 13.F, .ap_ = 12.F};
  const Ammo ammo{.name_ = "GLIDING-RKG", .mass_ = 1.4F, .drag_ = 0.1F, .lift_ = 1.0F};
  const Coord target{1034.0F, 432.0F, 0.0F};

  FirePosition res{};

  ComputationResult status = calc_fire_position(drone, ammo, target, res);

  EXPECT_NEAR(res.fire_.x_, 966.534F, 0.01F);
  EXPECT_NEAR(res.fire_.y_, 404.519F, 0.01F);
  EXPECT_EQ(res.fire_.z_, drone.position_.z_);

  EXPECT_NEAR(res.intermidiate_.x_, 966.534F, 0.01F);
  EXPECT_NEAR(res.intermidiate_.y_, 404.519F, 0.01F);
  EXPECT_EQ(res.intermidiate_.z_, drone.position_.z_);

  EXPECT_EQ(res.has_intermidiate_, false);
  EXPECT_EQ(status, ComputationResult::OK);
}

TEST(DroneBallisticCalc, withIntermidiatePosition_ammoRKG_3)
{
  const Drone drone{.position_{543.F, 232.F, 120.F}, .at_ = 13.F, .ap_ = 12.F};
  const Ammo ammo{.name_ = "RKG-3", .mass_ = 1.2F, .drag_ = 0.1F, .lift_ = 0.0F};
  const Coord target{553.0F, 242.0F, 0.0F};

  FirePosition res{};

  ComputationResult status = calc_fire_position(drone, ammo, target, res);

  EXPECT_NEAR(res.fire_.x_, 513.085F, 0.01F);
  EXPECT_NEAR(res.fire_.y_, 202.085F, 0.01F);
  EXPECT_EQ(res.fire_.z_, drone.position_.z_);

  EXPECT_NEAR(res.intermidiate_.x_, 504.600F, 0.01F);
  EXPECT_NEAR(res.intermidiate_.y_, 193.600F, 0.01F);
  EXPECT_EQ(res.intermidiate_.z_, drone.position_.z_);

  EXPECT_EQ(res.has_intermidiate_, true);
  EXPECT_EQ(status, ComputationResult::OK);
}

TEST(DroneBallisticCalc, withIntermidiatePosition_ammoM67)
{
  const Drone drone{.position_{543.F, 232.F, 120.F}, .at_ = 13.F, .ap_ = 12.F};
  const Ammo ammo{.name_ = "M67", .mass_ = 0.6F, .drag_ = 0.1F, .lift_ = 0.0F};
  const Coord target{533.0F, 232.0F, 0.0F};

  FirePosition res{};

  ComputationResult status = calc_fire_position(drone, ammo, target, res);

  EXPECT_NEAR(res.intermidiate_.x_, 597.504F, 0.01F);
  EXPECT_NEAR(res.intermidiate_.y_, 232.000F, 0.01F);
  EXPECT_EQ(res.intermidiate_.z_, drone.position_.z_);

  EXPECT_NEAR(res.fire_.x_, 585.504F, 0.01F);
  EXPECT_NEAR(res.fire_.y_, 232.000F, 0.01F);
  EXPECT_EQ(res.fire_.z_, drone.position_.z_);

  EXPECT_EQ(res.has_intermidiate_, true);
  EXPECT_EQ(status, ComputationResult::OK);
}

TEST(DroneBallisticCalc, withIntermidiatePosition_DroneCoordSameAsTargetCoords_ammoM67)
{
  const Drone drone{.position_{544.F, 233.F, 120.F}, .at_ = 13.F, .ap_ = 12.F};
  const Ammo ammo{.name_ = "M67", .mass_ = 0.6F, .drag_ = 0.1F, .lift_ = 0.0F};
  const Coord target{544.0F, 233.0F, 0.0F};

  FirePosition res{};

  ComputationResult status = calc_fire_position(drone, ammo, target, res);

  EXPECT_NEAR(res.intermidiate_.x_, 608.503F, 0.01F);
  EXPECT_NEAR(res.intermidiate_.y_, 233.F, 0.01F);
  EXPECT_EQ(res.intermidiate_.z_, drone.position_.z_);

  EXPECT_NEAR(res.fire_.x_, 596.503F, 0.01F);
  EXPECT_NEAR(res.fire_.y_, 233.F, 0.01F);
  EXPECT_EQ(res.fire_.z_, drone.position_.z_);

  EXPECT_EQ(res.has_intermidiate_, true);
  EXPECT_EQ(status, ComputationResult::OK);
}

TEST(DroneBallisticCalc, withDroneToHight_DroneToHighResult)
{
  const Drone drone{.position_{544.F, 233.F, 500.F}, .at_ = 13.F, .ap_ = 12.F};
  const Ammo ammo{.name_ = "M67", .mass_ = 0.6F, .drag_ = 0.1F, .lift_ = 0.0F};
  const Coord target{544.0F, 233.0F, 0.0F};

  FirePosition res{};

  ComputationResult status = calc_fire_position(drone, ammo, target, res);

  EXPECT_EQ(status, ComputationResult::AltitudeExceeded);
}
