#include <cstdint>
#include <rclcpp/rclcpp.hpp>
#include <algorithm>
#include <iterator>
#include <memory>
#include <optional>
#include <string>

#include "underground_world/msg/local_scan.hpp"
#include "underground_world/msg/move_command.hpp"
#include "underground_world/msg/student_status.hpp"
#include "underground_world/srv/payload_trigger.hpp"
#include "underground_world/pathfinder_service.hpp"
#include "underground_world/scenario.hpp"
#include "underground_world/world_model.hpp"

namespace {
using underground_world::msg::CellObservation;
using underground_world::msg::LocalScan;
using underground_world::msg::MoveCommand;
using underground_world::msg::StudentStatus;
using underground_world::srv::PayloadTrigger;

constexpr auto kScanTopic = "/robot/local_scan";
constexpr auto kMoveTopic = "/robot/cmd_move";
constexpr auto kStatusTopic = "/student/status";
constexpr auto kTriggerService = "/payload/trigger";

underground_world::CellKind cell_kind_from_string(const std::string& cell_type)
{
  if (cell_type == ".") {
    return underground_world::CellKind::Free;
  }
  if (cell_type == "S") {
    return underground_world::CellKind::Start;
  }
  if (cell_type == "C") {
    return underground_world::CellKind::Contact;
  }
  if (cell_type == "x") {
    return underground_world::CellKind::ProcessedContact;
  }
  return underground_world::CellKind::Wall;
}

underground_world::ObservedCell read_cell_msg(const CellObservation& cell)
{
  underground_world::ObservedCell observed;
  observed.position = underground_world::Position{cell.x, cell.y};
  observed.kind = cell_kind_from_string(cell.cell_type);
  observed.contact_id = cell.contact_id;
  return observed;
}

underground_world::LocalScanData read_scan_msg(const LocalScan& scn_msg)
{
  underground_world::LocalScanData scan;
  scan.scenario_name = scn_msg.scenario_name;
  scan.robot = underground_world::Position{scn_msg.robot_x, scn_msg.robot_y};

  scan.cells.reserve(scn_msg.cells.size());
  std::transform(scn_msg.cells.begin(), scn_msg.cells.end(), std::back_inserter(scan.cells), read_cell_msg);

  return scan;
}

std::optional<MoveCommand> make_move_msg(const underground_world::Move move)
{
  MoveCommand msg;
  switch (move) {
    case underground_world::Move::UP:
      msg.direction = MoveCommand::UP;
      break;
    case underground_world::Move::DOWN:
      msg.direction = MoveCommand::DOWN;
      break;
    case underground_world::Move::LEFT:
      msg.direction = MoveCommand::LEFT;
      break;
    case underground_world::Move::RIGHT:
      msg.direction = MoveCommand::RIGHT;
      break;
    case underground_world::Move::NONE:
      return std::nullopt;
  }
  return msg;
}

StudentStatus make_status_msg(const underground_world::Status status)
{
  StudentStatus msg;
  switch (status) {
    case underground_world::Status::EXPLORING:
      msg.state = StudentStatus::EXPLORING;
      break;
    case underground_world::Status::ENGAGING:
      msg.state = StudentStatus::ENGAGING;
      break;
    case underground_world::Status::RETURNING:
      msg.state = StudentStatus::RETURNING;
      break;
    case underground_world::Status::DONE:
      msg.state = StudentStatus::DONE;
      break;
    case underground_world::Status::FAILED:
      msg.state = StudentStatus::FAILED;
      break;
  }
  return msg;
}
}  // namespace

class PathfinderMoveNode final : public rclcpp::Node {
public:
  PathfinderMoveNode()
    : Node("pathfinder_move_node")
  {
    const auto qos = rclcpp::QoS{10};

    scan_sub_ = this->create_subscription<LocalScan>(kScanTopic, qos, [this](const LocalScan& scn_msg) { on_scan_recieved(scn_msg); });
    move_pub_ = this->create_publisher<MoveCommand>(kMoveTopic, qos);
    status_pub_ = this->create_publisher<StudentStatus>(kStatusTopic, qos);
    trigger_client_ = this->create_client<PayloadTrigger>(kTriggerService);
  }

private:
  auto on_scan_recieved(const LocalScan& scn_msg) -> void
  {
    pathfinder_service_.chart(read_scan_msg(scn_msg));
    const auto step = pathfinder_service_.derive_step();

    if (step.enemy_at) {
      RCLCPP_INFO(get_logger(),
                  "ENGAGING contact_id=%d at (%d,%d) -> calling payload trigger",
                  step.enemy_at->contact_id,
                  step.enemy_at->position.x,
                  step.enemy_at->position.y);

      auto request = std::make_shared<PayloadTrigger::Request>();
      request->contact_id = step.enemy_at->contact_id;
      request->x = step.enemy_at->position.x;
      request->y = step.enemy_at->position.y;
      trigger_client_->async_send_request(request);
    }
    else {
      const auto move_msg = make_move_msg(step.move);
      if (move_msg.has_value()) {
        RCLCPP_INFO(get_logger(), "EXPLORING -> move direction=%u", static_cast<unsigned>(move_msg->direction));
        move_pub_->publish(*move_msg);
      }
      else {
        RCLCPP_INFO(get_logger(), "no move issued (status=%u)", static_cast<unsigned>(static_cast<std::uint8_t>(step.status)));
      }
    }
    status_pub_->publish(make_status_msg(step.status));
  }

  PathfinderService pathfinder_service_;

  rclcpp::Subscription<LocalScan>::SharedPtr scan_sub_;
  rclcpp::Publisher<MoveCommand>::SharedPtr move_pub_;
  rclcpp::Publisher<StudentStatus>::SharedPtr status_pub_;
  rclcpp::Client<PayloadTrigger>::SharedPtr trigger_client_;
};

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<PathfinderMoveNode>());
  rclcpp::shutdown();
  return 0;
}
