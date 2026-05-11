#include "telemetry.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>

// Debugging exercise notes:
// this file intentionally contains four runtime defects.
// The defects are related to malformed input shape, invalid numeric values,
// unsafe time deltas, and empty logs. Exact locations are not marked on purpose.

const int EXPECTED_FIELD_COUNT = 7;
const int MAX_LINE_LENGTH = 256;

int split_line(char line[], char* fields[], int max_fields)
{
  int count = 0;
  char* cursor = line;

  while (*cursor != '\0' && count < max_fields) {
    while (*cursor == ' ' || *cursor == '\t' || *cursor == '\n' || *cursor == '\r') {
      *cursor = '\0';
      ++cursor;
    }

    if (*cursor == '\0') {
      break;
    }

    fields[count] = cursor;
    ++count;

    while (*cursor != '\0' && *cursor != ' ' && *cursor != '\t' && *cursor != '\n' && *cursor != '\r') {
      ++cursor;
    }
  }

  return count;
}

long parse_long(const char* text)
{
  char* end = nullptr;
  const long value = std::strtol(text, &end, 10);

  if (end == text) {
    std::abort();
  }

  return value;
}

int parse_int(const char* text)
{
  return static_cast<int>(parse_long(text));
}

double parse_double(const char* text)
{
  char* end = nullptr;
  const double value = std::strtod(text, &end);

  if (end == text) {
    std::abort();
  }

  return value;
}

Error validate_lines(char* fields[], int field_count)
{
  if (field_count != EXPECTED_FIELD_COUNT) {
    return Error::MissingArguments;
  }
  for (int i = 0; i < field_count; i++) {
    char* ptr = fields[i];
    if (*ptr == '-') {
      if (*(ptr + 1) == '\0') {
        return Error::WrongFormat;
      }
      ptr++;
    }
    while (*ptr != '\0') {
      if (*ptr != '.' && (*ptr < '0' || *ptr > '9')) {
        return Error::WrongFormat;
      }
      ptr++;
    }
  }
  return Error::OK;
}

Error validate_frame(const Frame& fr)
{
  if (fr.voltage_v <= 0) {
    return Error::BadVoltage;
  }
  if (fr.temperature_c < -40 || fr.temperature_c > 120) {
    return Error::BadTemperature;
  }
  if (fr.gps_fix < 0 || fr.gps_fix > 1) {
    return Error::BadGPS;
  }
  if (fr.satellites < 0) {
    return Error::BadSatelites;
  }
  return Error::OK;
}

Error validate_frames(const Frame& prev, const Frame& curr)
{
  if (curr.seq != prev.seq + 1) {
    return Error::BadSeq;
  }
  if (prev.timestamp_ms >= curr.timestamp_ms) {
    return Error::BadTimestamp;
  }
  return Error::OK;
}

Error parse_frame(char line[], Frame& frame)
{
  char* fields[EXPECTED_FIELD_COUNT] = {};
  const int field_count = split_line(line, fields, EXPECTED_FIELD_COUNT);

  Error is_line_valid = validate_lines(fields, field_count);

  if (is_line_valid > Error::OK) {
    return is_line_valid;
  }

  frame.timestamp_ms = parse_long(fields[0]);
  frame.seq = parse_int(fields[1]);
  frame.voltage_v = parse_double(fields[2]);
  frame.current_a = parse_double(fields[3]);
  frame.temperature_c = parse_double(fields[4]);
  frame.gps_fix = parse_int(fields[5]);
  frame.satellites = parse_int(fields[6]);

  Error is_frame_valid = validate_frame(frame);

  if (is_frame_valid > Error::OK) {
    return is_frame_valid;
  }

  return Error::OK;
}

double compute_frame_rate_hz(const Frame frames[], int frame_count)
{
  if (frame_count < 2) {
    return 0.0;
  }
  const long elapsed_ms = frames[frame_count - 1].timestamp_ms - frames[0].timestamp_ms;

  if (elapsed_ms == 0) {
    return 0.0;
  }

  return static_cast<double>((frame_count - 1) * 1000 / elapsed_ms);
}

Result read_frames(const char* path, Frame out_frames[], int* out_size, int max_frames)
{
  std::ifstream input{path};
  if (!input) {
    return {Error::MissingFile, -1};
  }

  int frame_count = 0;
  char line[MAX_LINE_LENGTH];

  while (input.getline(line, MAX_LINE_LENGTH)) {
    if (line[0] == '\0') {
      continue;
    }

    if (frame_count < max_frames) {
      Frame fr{};
      Error result = parse_frame(line, fr);

      if (result > Error::OK) {
        return {result, frame_count};
      }
      if (frame_count > 0) {
        Error is_monotonic = validate_frames(out_frames[frame_count - 1], fr);
        if (is_monotonic > Error::OK) {
          return {is_monotonic, frame_count};
        }
      }

      out_frames[frame_count] = fr;
      ++frame_count;
    }
  }

  *out_size = frame_count;

  return {Error::OK, -1};
}

Summary summarize(const Frame frames[], int frame_count)
{
  Summary summary{};

  if (frame_count == 0) {
    return summary;
  }

  summary.frames_total = frame_count;
  summary.frames_valid = frame_count;
  summary.voltage_min = frames[0].voltage_v;
  summary.voltage_max = frames[0].voltage_v;
  summary.low_voltage_frames = 0;

  double temperature_sum = 0.0;

  for (int i = 0; i < frame_count; ++i) {
    if (frames[i].voltage_v < summary.voltage_min) {
      summary.voltage_min = frames[i].voltage_v;
    }

    if (frames[i].voltage_v > summary.voltage_max) {
      summary.voltage_max = frames[i].voltage_v;
    }

    temperature_sum += frames[i].temperature_c;

    if (frames[i].voltage_v < 22.0) {
      ++summary.low_voltage_frames;
    }
  }

  const int temperature_tenths = static_cast<int>(temperature_sum * 10.0) / frame_count;
  summary.temperature_avg = static_cast<double>(temperature_tenths) / 10.0;
  summary.frame_rate_hz = compute_frame_rate_hz(frames, frame_count);
  return summary;
}

void print_summary(const Summary& summary)
{
  std::cout << "frames_total " << summary.frames_total << '\n';
  std::cout << "frames_valid " << summary.frames_valid << '\n';
  std::cout << "voltage_min " << summary.voltage_min << '\n';
  std::cout << "voltage_max " << summary.voltage_max << '\n';
  std::cout << "temperature_avg " << summary.temperature_avg << '\n';
  std::cout << "low_voltage_frames " << summary.low_voltage_frames << '\n';
  std::cout << "frame_rate_hz " << summary.frame_rate_hz << '\n';
}
