#include "antidrone_turret/msg/gimbal_command.hpp"
#include "antidrone_turret/msg/target.hpp"
#include "antidrone_turret/msg/servo_command.hpp"
#include "antidrone_turret/target_track_decisions.hpp"

#include <cstdint>
#include <rclcpp/executors.hpp>
#include <rclcpp/node.hpp>

namespace {
constexpr auto kTargetTopic = "/perception/target";
constexpr auto kGimbalTopic = "/gimbal/cmd";
constexpr auto kServoTopic = "/servo/cmd";

using ServoType = antidrone_turret::msg::ServoCommand;
using GimbalType = antidrone_turret::msg::GimbalCommand;
using TargetType = antidrone_turret::msg::Target;

ServoType to_servo_command(const antidrone_turret::ServoParameters& servo_parameters)
{
  return ServoType()
    .set__direction(static_cast<int8_t>(servo_parameters.command))
    .set__target_x(servo_parameters.target_x)
    .set__error_x(servo_parameters.error_x);
}

GimbalType to_gimbal_command(const antidrone_turret::GimbalParameters& gimbal_parameters)
{
  return GimbalType()
    .set__direction(static_cast<int8_t>(gimbal_parameters.command))
    .set__target_y(gimbal_parameters.target_y)
    .set__error_y(gimbal_parameters.error_y);
}
}  // namespace

class TurretControllerNode final : public rclcpp::Node {
public:
  TurretControllerNode()
    : rclcpp::Node("turret_controller_node")
  {
    const auto confidence_threshold = declare_parameter<double>("confidence_threshold", 0.8);
    const auto max_distance_m = declare_parameter<double>("max_distance_m", 30.0F);

    const auto screen_width = declare_parameter<double>("screen_width", 640.0);
    const auto screen_height = declare_parameter<double>("screen_height", 480.0);

    decision_config_ = {static_cast<float>(confidence_threshold), static_cast<float>(max_distance_m)};
    screen_resolution_config_ = {static_cast<float>(screen_width), static_cast<float>(screen_height)};

    servo_publisher_ = this->create_publisher<ServoType>(kServoTopic, 10);
    gimbal_publisher_ = this->create_publisher<GimbalType>(kGimbalTopic, 10);

    subscription_ =
      this->create_subscription<TargetType>(kTargetTopic, 10, [this](const TargetType& msg_tgt) { on_target_recieved(msg_tgt); });
  }

private:
  auto on_target_recieved(const TargetType& msg_tgt) -> void
  {
    const auto servo = antidrone_turret::calculate_servo_parameters(screen_resolution_config_, msg_tgt.x);
    const auto gimbal = antidrone_turret::calculate_gimbal_parameters(screen_resolution_config_, msg_tgt.y);

    servo_publisher_->publish(to_servo_command(servo));
    gimbal_publisher_->publish(to_gimbal_command(gimbal));
  }

  antidrone_turret::ScreenResolutionConfig screen_resolution_config_;
  antidrone_turret::DecisionConfig decision_config_;

  rclcpp::Subscription<TargetType>::SharedPtr subscription_;
  rclcpp::Publisher<GimbalType>::SharedPtr gimbal_publisher_;
  rclcpp::Publisher<ServoType>::SharedPtr servo_publisher_;
};

int main(int argc, char* argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<TurretControllerNode>());
  rclcpp::shutdown();
  return 0;
}