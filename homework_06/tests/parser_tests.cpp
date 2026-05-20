#include "dto.hpp"
#include "parser.hpp"

#include <gtest/gtest.h>
#include <sstream>

TEST(DroneBallisticParser, line_isValid_thenOk)
{
  Drone drone{};
  Ammo ammo{};
  Coord target{};

  std::istringstream stream{"543 232 120 543 232 13 12 M67"};

  ParsingResult result = parse(stream, drone, ammo, target);

  EXPECT_EQ(drone.position_.x_, 543.F);
  EXPECT_EQ(drone.position_.y_, 232.F);
  EXPECT_EQ(drone.position_.z_, 120.F);
  EXPECT_EQ(drone.at_, 13.F);
  EXPECT_EQ(drone.ap_, 12.F);

  EXPECT_EQ(target.x_, 543.F);
  EXPECT_EQ(target.y_, 232.F);
  EXPECT_EQ(target.z_, 0.F);

  // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay): EXPECT_STREQ requires char*, decay unavoidable
  EXPECT_STREQ(ammo.name_, "M67");
  EXPECT_EQ(ammo.mass_, 0.6F);
  EXPECT_EQ(ammo.drag_, 0.10F);
  EXPECT_EQ(ammo.lift_, 0.0F);

  EXPECT_EQ(result, ParsingResult::OK);
}

TEST(DroneBallisticParser, line_containsWrongCharacters_thenError)
{
  Drone drone{};
  Ammo ammo{};
  Coord target{};

  std::istringstream stream{"abc 232 120 543 232 13 12 M67"};

  ParsingResult result = parse(stream, drone, ammo, target);

  EXPECT_EQ(result, ParsingResult::Malformed);
}

TEST(DroneBallisticParser, line_containsUnknownAmmo_thenError)
{
  Drone drone{};
  Ammo ammo{};
  Coord target{};

  std::istringstream stream{"123 232 120 543 232 13 12 FOO"};

  ParsingResult result = parse(stream, drone, ammo, target);

  EXPECT_EQ(result, ParsingResult::UnknownAmmo);
}

TEST(DroneBallisticParser, line_containsBadAltitude_thenError)
{
  Drone drone{};
  Ammo ammo{};
  Coord target{};

  std::istringstream stream{"123 232 -10 543 232 13 12 M67"};

  ParsingResult result = parse(stream, drone, ammo, target);

  EXPECT_EQ(result, ParsingResult::AltitudeOutOfRange);
}

TEST(DroneBallisticParser, line_containsBadAttackSpeed_thenError)
{
  Drone drone{};
  Ammo ammo{};
  Coord target{};

  std::istringstream stream{"123 232 123 543 232 0 12 M67"};

  ParsingResult result = parse(stream, drone, ammo, target);

  EXPECT_EQ(result, ParsingResult::AttackSpeedOutOfRange);
}

TEST(DroneBallisticParser, line_containsBadAccelerationPath_thenError)
{
  Drone drone{};
  Ammo ammo{};
  Coord target{};

  std::istringstream stream{"123 232 123 543 232 13 0 M67"};

  ParsingResult result = parse(stream, drone, ammo, target);

  EXPECT_EQ(result, ParsingResult::AccelerationPathOutOfRange);
}

TEST(DroneBallisticParser, line_isEmpty_thenError)
{
  Drone drone{};
  Ammo ammo{};
  Coord target{};

  std::istringstream stream{""};

  ParsingResult result = parse(stream, drone, ammo, target);

  EXPECT_EQ(result, ParsingResult::Malformed);
}

TEST(DroneBallisticParser, line_lackOfArguments_thenError)
{
  Drone drone{};
  Ammo ammo{};
  Coord target{};

  std::istringstream stream{"123 232 123 543 M67"};

  ParsingResult result = parse(stream, drone, ammo, target);

  EXPECT_EQ(result, ParsingResult::Malformed);
}