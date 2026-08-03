#include "FileResultReporter.hpp"

#include <filesystem>
#include <fstream>

FileResultReporter::FileResultReporter(std::string path)
  : path_(std::move(path))
{
}

bool FileResultReporter::save(const std::vector<SimulationStep>& steps, const std::string& student_id, const std::string& test_id)
{
  const nlohmann::ordered_json report = to_json(steps, student_id, test_id);

  std::ofstream ofs(path_);
  ofs << report.dump(2);
  return static_cast<bool>(ofs);
}

bool FileResultReporter::check(const std::string& /*student_id*/, const std::string& /*test_id*/) const
{
  return std::filesystem::exists(path_);
}
