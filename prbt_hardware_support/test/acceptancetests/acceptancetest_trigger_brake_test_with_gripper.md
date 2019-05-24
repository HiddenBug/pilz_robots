<!--
Copyright (c) 2019 Pilz GmbH & Co. KG

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
-->

# Acceptance Test for the braketest feature
These acceptance tests check that the real robot system reacts to a BrakeTest service call properly.

## Prerequisites
  - Properly connect and startup the robot and power cabinet containing the PNoz-Multi.
    Make sure an emergency stop is within reach.

### Test Sequence
  1. Press the acknowledge button. Make sure the green light on the power cabinet is on.
     Run `roslaunch prbt_moveit_config moveit_planning_execution.launch sim:=False pipeline:=pilz_command_planner gripper:=pg70`
  2. Run `roslaunch prbt_hardware_support canopen_braketest_adapter_node.launch`
  3. Run `roslaunch prbt_hardware_support brake_test_executor_node.launch`
  4. Run `rosservice call /prbt/execute_braketest`
  5. Perform a long robot motion via Rviz and run `rosservice call /prbt/execute_braketest`
### Expected Results
  1. The robot starts properly and is moveable via Rviz.
  2. Node starts properly
  3. Node starts properly
  4. The brake test is executed for all joints ("Clicks" should be audible in a sequential order).
     The service responds with correct result code (for more information run `rosmsg show BrakeTestErrorCodes`).
  5. No brake test is executed. The service responds with correct result code and a descriptive error message.