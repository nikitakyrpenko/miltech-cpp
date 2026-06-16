#include "interfaces/IDroneProvider.hpp"
#include "models/Coord.hpp"
#include "models/Simulation.hpp"
#include "service/interface/state/IState.hpp"
#include "service/interfaces/IConfigLoader.hpp"
#include "service/state/StateStopped.hpp"

#include <memory>

class DroneProvider : public IDroneProvider {
private:
  Drone drone_;
  const IState* drone_state_ = StateStopped::get_instance();

  const Ammo ammo_;
  const Simulation simulation_;

  Task task_{-1, {}};
  bool visited_intermidiate_{false};

  Coord previous_position_;

public:
  DroneProvider(const std::shared_ptr<IConfigLoader> config);

  inline const Drone& get_drone() const override { return drone_; }
  inline const Ammo& get_ammo() const override { return ammo_; }
  inline const Simulation& get_simulation() const override { return simulation_; }

  inline std::string get_current_state() const override { return drone_state_->name(); }
  const Coord get_active_coord() const override;

  inline bool has_task_contains_intermidiate() const override { return task_.solution_.intermididate_.has_value(); }
  bool has_task_completed() const override;

  const Drone& execute(const Task& task, float dt) override;

  ~DroneProvider() override;
};