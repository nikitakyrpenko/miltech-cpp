#include "UartDronePhysics.hpp"
#include "UartLink.hpp"
#include "service/state/StateStopped.hpp"

#include <cmath>

UartDronePhysics::UartDronePhysics(std::shared_ptr<UartLink> link,
                                   const IConfigLoader& config_loader,
                                   std::shared_ptr<SynchronizedQueue<DroneCommand>> command_channel,
                                   float poll_period)
  : ScheduledWorker(0.05F)
  , link_(std::move(link))
  , telemetry_channel_(link_->telemetry_channel())
  , command_channel_(std::move(command_channel))
  , spec_(config_loader.get_config().attack_speed_,
          config_loader.get_config().acceleration_path_,
          config_loader.get_config().angular_speed_,
          config_loader.get_config().turn_threshold_)
{
}

const IState* UartDronePhysics::state_from_mode(DroneMode mode)
{
  switch (mode) {
    case DroneMode::ACCELERATING:
      return StateAccelerating::get_instance();
    case DroneMode::DECELERATING:
      return StateDecelerating::get_instance();
    case DroneMode::TURNING:
      return StateTurning::get_instance();
    case DroneMode::MOVING:
      return StateMoving::get_instance();
    default:
      return StateStopped::get_instance();
  }
}

void UartDronePhysics::apply(const dlink::Telemetry& t)
{
  std::lock_guard<std::mutex> l(tel_mtx_);
  if (!telemetry_.has_value()) {
    telemetry_.emplace(Coord{t.x, t.y}, t.dir, t.z);
  }
  telemetry_->set_position(Coord{t.x, t.y});
  telemetry_->set_current_speed(t.speed);
  telemetry_->set_current_direction(t.dir);
  telemetry_->set_elapsed(static_cast<float>(t.t_ms) / 1000.0F);
  telemetry_->set_altitude(t.z);
}

dlink::Control UartDronePhysics::to_control(const DroneCommand& cmd) const
{
  std::lock_guard<std::mutex> l(tel_mtx_);
  const float delta =
    std::atan2(std::sin(cmd.dir - telemetry_->get_current_direction()), std::cos(cmd.dir - telemetry_->get_current_direction()));

  const float accel = (cmd.state == DroneMode::ACCELERATING || cmd.state == DroneMode::MOVING) ? 1.0F : -1.0F;
  const float turn_rate = (std::abs(delta) < 1e-4F) ? 0.0F : std::copysign(1.0F, delta);

  return {accel, turn_rate};
}

void UartDronePhysics::tick()
{
  if (auto t = telemetry_channel_.drain_to_last()) {
    apply(*t);
  }
  if (auto cmd = command_channel_->drain_to_last()) {
    step(*cmd, 0.F);
  }

  if (is_ready()) {
    link_->send(to_control(get_active_command()));
  }
}

bool UartDronePhysics::is_ready() const
{
  std::lock_guard<std::mutex> l(tel_mtx_);
  return telemetry_.has_value();
}

void UartDronePhysics::step(const DroneCommand& command, float /*dt*/)
{
  std::lock_guard<std::mutex> l(command_mtx_);
  command_ = command;
}

const DroneTelemetry UartDronePhysics::get_telemetry() const
{
  std::lock_guard<std::mutex> l(tel_mtx_);
  if (!telemetry_) {
    throw std::runtime_error("UartDronePhysics: telemetry not yet received");
  }
  return *telemetry_;
}

const DroneSpec UartDronePhysics::get_spec() const
{
  return spec_;
}

const DroneCommand UartDronePhysics::get_active_command() const
{
  std::lock_guard<std::mutex> l(command_mtx_);
  return command_;
}

const IState* UartDronePhysics::get_state() const
{
  return state_from_mode(get_active_command().state);
}
