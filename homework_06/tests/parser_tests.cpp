#include "parser.hpp"

#include <gtest/gtest.h>
#include <sstream>

TEST(DroneBallisticParser, line_isValid_thenOk)
{
  Drone d{};
  Ammo a{};
  Coord t{};

  std::istringstream stream{"abc 232 120 543 232 13 12 M67"};

  Result result = parse(stream, d, a, t);

  EXPECT_EQ(d.position.x, 543.f);
  EXPECT_EQ(d.position.y, 232.f);
  EXPECT_EQ(d.position.z, 120.f);
  EXPECT_EQ(d.at, 13.f);
  EXPECT_EQ(d.ap, 12.f);

  EXPECT_EQ(t.x, 543.f);
  EXPECT_EQ(t.y, 232.f);
  EXPECT_EQ(t.z, 0.f);

  EXPECT_STREQ(a.name, "M67");
  EXPECT_EQ(a.mass, 0.6f);
  EXPECT_EQ(a.drag, 0.10f);
  EXPECT_EQ(a.lift, 0.0f);

  EXPECT_EQ(result, Result::OK);
}

TEST(DroneBallisticParser, line_containsWrongCharacters_thenError)
{
  Drone d{};
  Ammo a{};
  Coord t{};

  std::istringstream stream{"abc 232 120 543 232 13 12 M67"};

  Result result = parse(stream, d, a, t);

  EXPECT_EQ(result, Result::FileParsingError);
}

TEST(DroneBallisticParser, line_containsUnknownAmmo_thenError)
{
  Drone d{};
  Ammo a{};
  Coord t{};

  std::istringstream stream{"123 232 120 543 232 13 12 FOO"};

  Result result = parse(stream, d, a, t);

  EXPECT_EQ(result, Result::UnknownAmmo);
}

TEST(DroneBallisticParser, line_containsBadAltitude_thenError)
{
  Drone d{};
  Ammo a{};
  Coord t{};

  std::istringstream stream{"123 232 -10 543 232 13 12 M67"};

  Result result = parse(stream, d, a, t);

  EXPECT_EQ(result, Result::BadAltitude);
}

TEST(DroneBallisticParser, line_containsBadAttackSpeed_thenError)
{
  Drone d{};
  Ammo a{};
  Coord t{};

  std::istringstream stream{"123 232 123 543 232 0 12 M67"};

  Result result = parse(stream, d, a, t);

  EXPECT_EQ(result, Result::BadAttackSpeed);
}

TEST(DroneBallisticParser, line_containsBadAccelerationPath_thenError)
{
  Drone d{};
  Ammo a{};
  Coord t{};

  std::istringstream stream{"123 232 123 543 232 13 0 M67"};

  Result result = parse(stream, d, a, t);

  EXPECT_EQ(result, Result::BadAccelerationPath);
}

TEST(DroneBallisticParser, line_isEmpty_thenError)
{
  Drone d{};
  Ammo a{};
  Coord t{};

  std::istringstream stream{""};

  Result result = parse(stream, d, a, t);

  EXPECT_EQ(result, Result::FileParsingError);
}

TEST(DroneBallisticParser, line_lackOfArguments_thenError)
{
  Drone d{};
  Ammo a{};
  Coord t{};

  std::istringstream stream{"123 232 123 543 M67"};

  Result result = parse(stream, d, a, t);

  EXPECT_EQ(result, Result::FileParsingError);
}