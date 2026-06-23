#include "service/DronePhysics.hpp"

#include <chrono>
#include <mutex>
#include <thread>

void DronePhysics::step(const DroneCommand& command, float dt)
{
  // calculate direction
  float delta =
    std::atan2(std::sin(command.dir - telemetry_.get_current_direction()), std::cos(command.dir - telemetry_.get_current_direction()));

  float rot_step = spec_.get_angular_speed() * dt;
  if (std::abs(delta) <= rot_step) {
    telemetry_.set_current_direction(command.dir);
  }
  else {
    telemetry_.set_current_direction(telemetry_.get_current_direction() + std::copysign(rot_step, delta));
  }

  // calculate speed
  float speed = telemetry_.get_current_speed();
  switch (command.state) {
    case DroneMode::ACCELERATING:
      speed = std::min(speed + spec_.get_acceleration() * dt, spec_.get_attack_speed());
      break;
    case DroneMode::DECELERATING:
      speed = std::max(speed - spec_.get_acceleration() * dt, 0.0F);
      break;
    case DroneMode::MOVING:
      speed = spec_.get_attack_speed();
      break;
    case DroneMode::TURNING:
    case DroneMode::STOPPED:
      speed = 0.0F;
      break;
  }
  telemetry_.set_current_speed(speed);

  // update coordinates
  telemetry_.set_position(telemetry_.get_position() +
                          Coord{std::cos(telemetry_.get_current_direction()), std::sin(telemetry_.get_current_direction())} * (speed * dt));

  // update clock
  telemetry_.set_elapsed(telemetry_.elapsed() + dt);
}

void DronePhysics::interrupt()
{
  running_ = false;
}

void DronePhysics::start(std::latch& latch)
{
  running_ = true;
  worker_ = std::thread([this, &latch]() {
    latch.arrive_and_wait();

    using clock = std::chrono::steady_clock;
    const auto period = std::chrono::duration_cast<clock::duration>(std::chrono::duration<float>(physics_time_step / timescale_));

    auto next = clock::now();
    while (running_) {
      if (channel_->size() > 0) {
        std::lock_guard<std::mutex> command_guard(command_mtx_);
        if (auto command = channel_->drain_to_last())
          command_ = *command;
      }
      {
        std::lock_guard<std::mutex> telemetry_guard(tel_mtx_);
        step(command_, physics_time_step);
      }
      next += period;
      std::this_thread::sleep_until(next);
    }
  });
}

const DroneSpec DronePhysics::get_spec() const
{
  return spec_;
}

const DroneTelemetry DronePhysics::get_telemetry() const
{
  std::lock_guard<std::mutex> telemetry_guard(tel_mtx_);
  return telemetry_;
}

const DroneCommand DronePhysics::get_active_command() const
{
  std::lock_guard<std::mutex> command_guard(command_mtx_);
  return command_;
}

DronePhysics::~DronePhysics()
{
  interrupt();

  if (worker_.joinable()) {
    worker_.join();
  }
}