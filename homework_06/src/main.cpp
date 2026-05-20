#include "ballistic.hpp"
#include "parser.hpp"

#include <cstddef>
#include <fstream>
#include <iostream>
#include <span>

int main(int argc, char** argv)
{
  std::span<char*> args{argv, static_cast<size_t>(argc)};

  if (args.size() != 2) {
    std::cerr << "usage: drone_ballistic_cli <input_path>\n";
    return 1;
  }

  std::ifstream file(args[1]);

  if (!file) {
    std::cerr << "file " << args[1] << " not found\n";
    return 1;
  }

  Drone drone{};
  Ammo ammo{};
  Coord target{};

  ParsingResult pr = parse(file, drone, ammo, target);

  switch (pr) {
    case ParsingResult::Malformed: {
      std::cerr << "file " << args[1] << " empty or malformed\n";
      return 1;
    }
    case ParsingResult::UnknownAmmo: {
      std::cerr << "file " << args[1] << " cannot recognize ammo\n";
      return 1;
    }
    case ParsingResult::AttackSpeedOutOfRange: {
      std::cerr << "file " << args[1] << " attack speed value zero or less\n";
      return 1;
    }
    case ParsingResult::AccelerationPathOutOfRange: {
      std::cerr << "file " << args[1] << " acceleration path value zero or less\n";
      return 1;
    }
    case ParsingResult::AltitudeOutOfRange: {
      std::cerr << "file " << args[1] << " altitude value zero or less\n";
      return 1;
    }
    case ParsingResult::OK: {
    }
  }

  FirePosition fire{};
  ComputationResult cr = calc_fire_position(drone, ammo, target, fire);

  switch (cr) {
    case ComputationResult::AltitudeExceeded: {
      std::cerr << "Cannot approximate calculate ammo time to fall - altitude to high\n";
      return 1;
    }
    case ComputationResult::OK: {
    }
  }

  std::ofstream out("output.txt");
  if (fire.has_intermidiate_) {
    out << fire.intermidiate_.x_ << " " << fire.intermidiate_.y_ << "\n";
  }
  out << fire.fire_.x_ << " " << fire.fire_.y_ << "\n";

  out.close();
  file.close();

  return 0;
}