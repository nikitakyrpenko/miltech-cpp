#!/usr/bin/env bash
# Records a proof rosbag for each mandatory scenario into homework_14/bags/<scenario>.
# Run inside the devcontainer AFTER building:
#   colcon build --packages-select underground_world
#   bash scripts/record_bags.sh

WS="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"   # .../homework_14/robot_ws
cd "$WS"

source /opt/ros/jazzy/setup.bash
source install/setup.bash

BAGS="$WS/../bags"
mkdir -p "$BAGS"

RECORD_SECONDS=12   # each scenario reaches SUCCESS in ~4s; recorder self-stops after this

# SIGINT a process group, wait for clean exit, SIGKILL if it lingers (used for the sim only).
stop_group() {
  local pgid="$1"
  kill -INT -"$pgid" 2>/dev/null
  for _ in $(seq 1 10); do
    kill -0 -"$pgid" 2>/dev/null || return 0
    sleep 0.5
  done
  kill -KILL -"$pgid" 2>/dev/null
}

for sc in training_corridor small_rooms branching_trench dead_end_bunker; do
  echo "==================== $sc ===================="
  rm -rf "$BAGS/$sc"

  # recorder: timeout delivers SIGINT after RECORD_SECONDS, so rosbag2 finalizes
  # (writes metadata.yaml) itself -- exactly like a manual Ctrl-C.
  timeout -s INT "$RECORD_SECONDS" ros2 bag record -a -o "$BAGS/$sc" >/dev/null 2>&1 &
  rec_pid=$!
  sleep 2   # recorder up before the sim's first publish

  # sim in its own session/process group so we can take down the whole node tree
  setsid ros2 launch underground_world system.launch.py scenario:="${sc}.yaml" >/dev/null 2>&1 &
  launch_pgid=$!

  wait "$rec_pid"            # blocks until the recorder's timeout fires and it finalizes
  stop_group "$launch_pgid"  # then shut the sim down

  echo "---- bag info: $sc ----"
  ros2 bag info "$BAGS/$sc"
  echo
done

echo "Done. Bags in $BAGS"
