#include "dto/TargetDTO.hpp"
#include "models/Target.hpp"
#include "service/interfaces/IConfigLoader.hpp"
#include "service/interfaces/ITargetProvider.hpp"

#include <memory>
#include <vector>

class TargetProvider : public ITargetProvider {
  const std::vector<Target> targets_;

public:
  TargetProvider(const std::shared_ptr<IConfigLoader> config);

  TargetProviderIterator begin() override;
  TargetProviderIterator end() override;

  const Target& get_target(int id) const override;
  int size() const override;

  ~TargetProvider() override;
};