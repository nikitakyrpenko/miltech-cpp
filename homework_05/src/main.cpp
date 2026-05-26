#include "telemetry.hpp"

#include <iostream>

int main(int argc, char** argv)
{
  // The executable expects exactly one telemetry log path.
  if (argc != 2) {
    std::cerr << "usage: telemetry_check <input_path>\n";
    return 1;
  }

  Frame frames[MAX_TELEMETRY_FRAMES];
  int frames_size{};

  const Result res = read_frames(argv[1], frames, &frames_size, MAX_TELEMETRY_FRAMES);

  switch (res.err) {
    case WrongFormat: {
      std::cerr << "error: invalid character combination at line " << res.at_line << std::endl;
      return 1;
    }
    case MissingFile: {
      std::cerr << "error: failed to open input file: " << argv[1] << '\n';
      return 1;
    }
    case MissingArguments: {
      std::cerr << "error: invalid frame at line " << res.at_line << " : expected " << 7 << " fields\n";
      return 1;
    }
    case BadVoltage: {
      std::cerr << "error: invalid voltage value at line " << res.at_line << std::endl;
      return 1;
    }
    case BadTemperature: {
      std::cerr << "error: invalid temperature value at line " << res.at_line << std::endl;
      return 1;
    }
    case BadGPS: {
      std::cerr << "error: invalid GPS value at line " << res.at_line << std::endl;
      return 1;
    }
    case BadSatelites: {
      std::cerr << "error: invalid satelite value at line " << res.at_line << std::endl;
      return 1;
    }
    case BadSeq: {
      std::cerr << "error: seq value does not increases monotonically at line " << res.at_line << std::endl;
      return 1;
    }
    case BadTimestamp: {
      std::cerr << "error: timestamp_ms value does not increases monotonically at line " << res.at_line << std::endl;
      return 1;
    }
    case EmptyFile: {
      std::cerr << "error: input file contains no valid frames\n";
      return 1;
    }
    case OK: {
    }
  }

  const Summary summary = summarize(frames, frames_size);
  print_summary(summary);

  return 0;
}
