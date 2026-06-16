#include "service/TargetProvider.hpp"
#include "service/TargetProviderIterator.hpp"
#include "service/interfaces/IConfigLoader.hpp"
#include "service/interfaces/ITargetProvider.hpp"

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

namespace {

std::vector<Target> create_targets(const TargetDTO& targets, const ConfigDTO& config)
{
  std::vector<Target> result;
  result.reserve(targets.positions_.size());

  for (size_t i{0}; i < targets.positions_.size(); i++) {
    Target t(static_cast<int>(i), targets.positions_.at(i), targets.time_steps_, config.target_array_timestep_);
    result.emplace_back(std::move(t));
  };

  return result;
}

}  // namespace

TargetProvider::TargetProvider(const std::shared_ptr<IConfigLoader> config)
  : targets_(create_targets(config->get_targets(), config->get_config()))
{
}

const Target& TargetProvider::get_target(int id) const
{
  return targets_.at(id);
}

int TargetProvider::size() const
{
  return targets_.size();
}

TargetProviderIterator TargetProvider::begin()
{
  return TargetProviderIterator(this, 0);
}

TargetProviderIterator TargetProvider::end()
{
  return TargetProviderIterator(this, this->size());
}

TargetProvider::~TargetProvider() {}