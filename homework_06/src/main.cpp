#include <fstream>
#include <iostream>

#include "ballistic.hpp"
#include "parser.hpp"

int main(int argc, char** argv)
{
  if (argc != 2) {
    std::cerr << "usage: drone_ballistic_cli <input_path>\n";
    return 1;
  }

  std::ifstream file(argv[1]);

  if (!file) {
    std::cerr << "file " << argv[1] << " not found\n";
    return 1;
  }

  Drone drone{};
  Ammo ammo{};
  Coord target{};

  ParsingResult pr = parse(file, drone, ammo, target);

  switch (pr) {
    case ParsingResult::Malformed: {
      std::cerr << "file " << argv[1] << " empty or malformed\n";
      return 1;
    }
    case ParsingResult::UnknownAmmo: {
      std::cerr << "file " << argv[1] << " cannot recognize ammo\n";
      return 1;
    }
    case ParsingResult::AttackSpeedOutOfRange: {
      std::cerr << "file " << argv[1] << " attack speed value zero or less\n";
      return 1;
    }
    case ParsingResult::AccelerationPathOutOfRange: {
      std::cerr << "file " << argv[1] << " acceleration path value zero or less\n";
      return 1;
    }
    case ParsingResult::AltitudeOutOfRange: {
      std::cerr << "file " << argv[1] << " altitude value zero or less\n";
      return 1;
    }
    case ParsingResult::OK: {
    }
  }

  FirePosition fire{};
  ComputationResult cr = calcFirePosition(drone, ammo, target, fire);

  switch (cr) {
    case ComputationResult::AltitudeExceeded: {
      std::cerr << "Cannot approximate calculate ammo time to fall - altitude to high\n";
      return 1;
    }
    case ComputationResult::OK: {
    }
  }

  std::ofstream out("output.txt");
  if (fire.hasIntermidiate) {
    out << fire.intermidiate.x << " " << fire.intermidiate.y << "\n";
  }
  out << fire.fire.x << " " << fire.fire.y << "\n";

  out.close();
  file.close();

  return 0;
}