#    COPYRIGHT (C) 2025 Mitsubishi Electric Corporation

#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at

#        http://www.apache.org/licenses/LICENSE-2.0

#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
# Contributor(s):
#   Cao Feng
#
# Description: After a robot has been loaded, this will execute a series of trajectories.

from launch import LaunchDescription
from launch.substitutions import PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare
from launch.substitutions import LaunchConfiguration
from launch.actions import DeclareLaunchArgument



def generate_launch_description():
    declared_arguments = []
    declared_arguments.append(
        DeclareLaunchArgument(
            "test_file",
            default_value="",
            description="Namespace of controller manager and controllers. This is useful for \
        multi-robot scenarios.",
        )
    )
    test_file = LaunchConfiguration("test_file")

    position_goals = PathJoinSubstitution(
        [FindPackageShare("melfa_bringup"), "config", test_file]
    )

    return LaunchDescription(
        [
            Node(
                package="ros2_controllers_test_nodes",
                executable="publisher_joint_trajectory_controller",
                name="publisher_joint_trajectory_controller",
                parameters=[position_goals],
                output="screen",
            )
        ]
    )
