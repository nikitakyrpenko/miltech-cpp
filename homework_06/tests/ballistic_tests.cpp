#include <gtest/gtest.h>

#include "ballistic.hpp"
TEST(TestCalcFirePosition, noIntermidiatePosition_ammoVOG_17)
{
  const Drone d{.position{180.f, 180.f, 100.f}, .at = 10.f, .ap = 10.f};
  const Ammo a{.name = "VOG-17", .mass = 0.35f, .drag = 0.07f, .lift = 0.0f};
  const Coord t{200.f, 200.f, 0.f};

  FirePosition res{};

  Result status = calcFirePosition(d, a, t, res);

  EXPECT_NEAR(res.fire.x, 173.759f, 0.01f);
  EXPECT_NEAR(res.fire.y, 173.759f, 0.01f);
  EXPECT_EQ(res.fire.z, d.position.z);

  EXPECT_NEAR(res.intermidiate.x, 166.688f, 0.01f);
  EXPECT_NEAR(res.intermidiate.y, 166.688f, 0.01f);
  EXPECT_EQ(res.intermidiate.z, d.position.z);

  EXPECT_EQ(res.hasIntermidiate, true);
  EXPECT_EQ(status, Result::OK);
}

TEST(TestCalcFirePosition, noIntermidiatePosition_ammoGLIDING_VOG)
{
  const Drone d{.position{0.f, 0.f, 100.f}, .at = 20.f, .ap = 50.f};
  const Ammo a{.name = "GLIDING-VOG", .mass = 0.45f, .drag = 0.10f, .lift = 1.0f};
  const Coord t{300.f, 300.f, 0.f};

  FirePosition res{};

  Result status = calcFirePosition(d, a, t, res);

  EXPECT_NEAR(res.fire.x, 242.711f, 0.01f);
  EXPECT_NEAR(res.fire.y, 242.711f, 0.01f);
  EXPECT_EQ(res.fire.z, d.position.z);

  EXPECT_NEAR(res.intermidiate.x, 242.711f, 0.01f);
  EXPECT_NEAR(res.intermidiate.y, 242.711f, 0.01f);
  EXPECT_EQ(res.intermidiate.z, d.position.z);

  EXPECT_EQ(res.hasIntermidiate, false);
  EXPECT_EQ(status, Result::OK);
}

TEST(TestCalcFirePosition, noIntermidiatePosition_ammoGLIDING_RKG)
{
  const Drone d{.position{543.f, 232.f, 120.f}, .at = 13.f, .ap = 12.f};
  const Ammo a{.name = "GLIDING-RKG", .mass = 1.4f, .drag = 0.1f, .lift = 1.0f};
  const Coord t{1034.0f, 432.0f, 0.0f};

  FirePosition res{};

  Result status = calcFirePosition(d, a, t, res);

  EXPECT_NEAR(res.fire.x, 966.534f, 0.01f);
  EXPECT_NEAR(res.fire.y, 404.519f, 0.01f);
  EXPECT_EQ(res.fire.z, d.position.z);

  EXPECT_NEAR(res.intermidiate.x, 966.534f, 0.01f);
  EXPECT_NEAR(res.intermidiate.y, 404.519f, 0.01f);
  EXPECT_EQ(res.intermidiate.z, d.position.z);

  EXPECT_EQ(res.hasIntermidiate, false);
  EXPECT_EQ(status, Result::OK);
}

TEST(TestCalcFirePosition, withIntermidiatePosition_ammoRKG_3)
{
  const Drone d{.position{543.f, 232.f, 120.f}, .at = 13.f, .ap = 12.f};
  const Ammo a{.name = "RKG-3", .mass = 1.2f, .drag = 0.1f, .lift = 0.0f};
  const Coord t{553.0f, 242.0f, 0.0f};

  FirePosition res{};

  Result status = calcFirePosition(d, a, t, res);

  EXPECT_NEAR(res.fire.x, 513.085f, 0.01f);
  EXPECT_NEAR(res.fire.y, 202.085f, 0.01f);
  EXPECT_EQ(res.fire.z, d.position.z);

  EXPECT_NEAR(res.intermidiate.x, 504.600f, 0.01f);
  EXPECT_NEAR(res.intermidiate.y, 193.600f, 0.01f);
  EXPECT_EQ(res.intermidiate.z, d.position.z);

  EXPECT_EQ(res.hasIntermidiate, true);
  EXPECT_EQ(status, Result::OK);
}

TEST(TestCalcFirePosition, withIntermidiatePosition_ammoM67)
{
  const Drone d{.position{543.f, 232.f, 120.f}, .at = 13.f, .ap = 12.f};
  const Ammo a{.name = "M67", .mass = 0.6f, .drag = 0.1f, .lift = 0.0f};
  const Coord t{533.0f, 232.0f, 0.0f};

  FirePosition res{};

  Result status = calcFirePosition(d, a, t, res);

  EXPECT_NEAR(res.intermidiate.x, 597.504f, 0.01f);
  EXPECT_NEAR(res.intermidiate.y, 232.000f, 0.01f);
  EXPECT_EQ(res.intermidiate.z, d.position.z);

  EXPECT_NEAR(res.fire.x, 585.504f, 0.01f);
  EXPECT_NEAR(res.fire.y, 232.000f, 0.01f);
  EXPECT_EQ(res.fire.z, d.position.z);

  EXPECT_EQ(res.hasIntermidiate, true);
  EXPECT_EQ(status, Result::OK);
}

TEST(TestCalcFirePosition, withIntermidiatePosition_DroneCoordSameAsTargetCoords_ammoM67)
{
  const Drone d{.position{544.f, 233.f, 120.f}, .at = 13.f, .ap = 12.f};
  const Ammo a{.name = "M67", .mass = 0.6f, .drag = 0.1f, .lift = 0.0f};
  const Coord t{544.0f, 233.0f, 0.0f};

  FirePosition res{};

  Result status = calcFirePosition(d, a, t, res);

  EXPECT_NEAR(res.intermidiate.x, 608.503f, 0.01f);
  EXPECT_NEAR(res.intermidiate.y, 233.f, 0.01f);
  EXPECT_EQ(res.intermidiate.z, d.position.z);

  EXPECT_NEAR(res.fire.x, 596.503f, 0.01f);
  EXPECT_NEAR(res.fire.y, 233.f, 0.01f);
  EXPECT_EQ(res.fire.z, d.position.z);

  EXPECT_EQ(res.hasIntermidiate, true);
  EXPECT_EQ(status, Result::OK);
}

TEST(TestCalcFirePosition, withDroneToHight_DroneToHighResult)
{
  const Drone d{.position{544.f, 233.f, 500.f}, .at = 13.f, .ap = 12.f};
  const Ammo a{.name = "M67", .mass = 0.6f, .drag = 0.1f, .lift = 0.0f};
  const Coord t{544.0f, 233.0f, 0.0f};

  FirePosition res{};

  Result status = calcFirePosition(d, a, t, res);

  EXPECT_EQ(status, Result::DroneToHigh);
}