#    COPYRIGHT (C) 2024 Mitsubishi Electric Corporation

#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at

#        http://www.apache.org/licenses/LICENSE-2.0

#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.

import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, RegisterEventHandler, ExecuteProcess, IncludeLaunchDescription
from launch.conditions import IfCondition, UnlessCondition
from launch.event_handlers import OnProcessExit
from launch.substitutions import Command, FindExecutable, LaunchConfiguration, PathJoinSubstitution
from launch.launch_description_sources import PythonLaunchDescriptionSource

from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare
from launch.event_handlers import OnProcessStart
from launch_ros.parameter_descriptions import ParameterFile

def generate_launch_description():
    # Declare arguments
    declared_arguments = []
    declared_arguments.append(
        DeclareLaunchArgument(
            'runtime_config_package',
            default_value='melfa_description',
            description='Package with the controller\'s configuration in "config" folder. \
        Usually the argument is not set, it enables use of a custom setup.',
        )
    )
    declared_arguments.append(
        DeclareLaunchArgument(
            'controllers_file',
            default_value='rh6frh5520_controllers.yaml',
            description='YAML file with the controllers configuration.',
        )
    )
    declared_arguments.append(
        DeclareLaunchArgument(
            'description_package',
            default_value='melfa_description',
            description='Description package with robot URDF/xacro files. Usually the argument \
            is not set, it enables use of a custom description.',
        )
    )
    declared_arguments.append(
        DeclareLaunchArgument(
            'description_file',
            default_value='rh6frh5520/rh6frh5520.urdf.xacro',
            description='URDF/XACRO description file with the robot.',
        )
    )
    declared_arguments.append(
        DeclareLaunchArgument(
            'prefix',
            default_value='',
            description='Prefix of the joint names, useful for multi-robot setup. \
                        If changed than also joint names in the controllers \
                        configuration have to be updated.',
        )
    )
    declared_arguments.append(
        DeclareLaunchArgument(
            'use_sim',
            default_value='false',
            description='Start robot in Gazebo simulation.',
        )
    )
    declared_arguments.append(
        DeclareLaunchArgument(
            'use_fake_hardware',
            default_value='true',
            description='Start robot with fake hardware mirroring command to its states.',
        )
    )
    declared_arguments.append(
        DeclareLaunchArgument(
            'robot_controller',
            default_value='rh6frh5520_controller',
            description='Robot controller to start.',
        )
    )
    declared_arguments.append(
        DeclareLaunchArgument(
            'start_rviz',
            default_value='true',
            description='Start RViz2 automatically with this launch file.',
        )
    )
    declared_arguments.append(
        DeclareLaunchArgument(
            'robot_ip',
            default_value='192.168.0.20',
            description='Robot IP.',
        )
    )
    declared_arguments.append(
        DeclareLaunchArgument(
            'robot_port',
            default_value='10000',
            description='Robot port.',
        )
    )
    declared_arguments.append(
        DeclareLaunchArgument(
            'initial_positions_file',
            default_value='initial_positions_rh6frh.yaml',
            description='Configuration file of robot initial positions for simulation.',
        )
    )
    declared_arguments.append(
        DeclareLaunchArgument(
            'controller_config',
            default_value='custom_io_params.yaml',
            description='Configuration file of controller limits.',
        )
    )
    declared_arguments.append(
        DeclareLaunchArgument(
            'controller_type',
            default_value="R",
            description='Select MELFA Controller Type : [R or Q or D]',
        )
    )
    declared_arguments.append(
        DeclareLaunchArgument(
            'launch_servo',
            default_value='false',
            description='Argument to activate controllers for servo',
        )
    )
    declared_arguments.append(
        DeclareLaunchArgument(
            'packet_lost_log',
            default_value='1',
            description='Enable/disable packet lost warning. DO NOTE DISABLE WHEN USING A REAL ROBOT.',
        )
    )
    
    # Initialize Arguments
    runtime_config_package = LaunchConfiguration('runtime_config_package')
    controllers_file = LaunchConfiguration('controllers_file')
    description_package = LaunchConfiguration('description_package')
    description_file = LaunchConfiguration('description_file')
    prefix = LaunchConfiguration('prefix')
    use_sim = LaunchConfiguration('use_sim')
    use_fake_hardware = LaunchConfiguration('use_fake_hardware')
    robot_controller = LaunchConfiguration('robot_controller')
    start_rviz = LaunchConfiguration('start_rviz')
    robot_ip = LaunchConfiguration('robot_ip')
    robot_port = LaunchConfiguration('robot_port')
    controller_type = LaunchConfiguration('controller_type')
    launch_servo = LaunchConfiguration('launch_servo')
    packet_lost_log = LaunchConfiguration('packet_lost_log')

    initial_positions_file = LaunchConfiguration('initial_positions_file')

    initial_positions_file = PathJoinSubstitution(
        [FindPackageShare(description_package), "config", initial_positions_file]
    )

    controller_config = LaunchConfiguration('controller_config')
    
    controller_config = PathJoinSubstitution(
        [FindPackageShare(description_package), "config", controller_config]
    )
    robot_controllers = PathJoinSubstitution(
        [
            FindPackageShare(runtime_config_package),
            'config',
            controllers_file,
        ]
    )
    # Get URDF via xacro
    robot_description_content = Command(
        [
            PathJoinSubstitution([FindExecutable(name='xacro')]),
            ' ',
            PathJoinSubstitution(
                [FindPackageShare(description_package), 'urdf', description_file]
            ),
            ' ',
            'prefix:=',
            prefix,
            ' ',
            'use_sim:=',
            use_sim,
            ' ',
            'use_fake_hardware:=',
            use_fake_hardware,
            ' ',
            'robot_ip:=',
            robot_ip,
            ' ',
            'robot_port:=',
            robot_port,
            ' ',
            'initial_positions_file:=',
            initial_positions_file,
            ' ',
            'controller_config:=',
            controller_config,
            ' ',
            'controller_type:=',
            controller_type,
            ' ',
            'launch_servo:=',
            launch_servo,
            ' ',
            "simulation_controllers:=",
            robot_controllers,
            ' ',
            "packet_lost_log:=",
            packet_lost_log,
        ]
    )
    robot_description = {'robot_description': robot_description_content}

    rviz_config_file = PathJoinSubstitution(
        [FindPackageShare(description_package), 'rviz', 'rh6frh5520.rviz']
    )

    control_node = Node(
        package='controller_manager',
        executable='ros2_control_node',
        parameters=[ParameterFile(robot_controllers, allow_substs=True)],
        remappings=[("~/robot_description", "robot_description"),],
        output={
            'stdout': 'screen',
            'stderr': 'screen',
        },
        condition=UnlessCondition(use_sim),

    )

    robot_state_pub_node = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        output='both',
        parameters=[robot_description],
    )

    rviz_node = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        output='log',
        arguments=['-d', rviz_config_file],
        condition=IfCondition(start_rviz),
    )

    joint_state_broadcaster_spawner = Node(
        package='controller_manager',
        executable='spawner',
        arguments=['joint_state_broadcaster', '--controller-manager', '/controller_manager'],
    )

    robot_controller_spawner = Node(
        package='controller_manager',
        executable='spawner',
        arguments=[robot_controller, '-c', '/controller_manager'],
    )

    gpio_controller_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["gpio_controller", "-c", "/controller_manager"],
        condition=UnlessCondition(use_fake_hardware) or UnlessCondition(use_sim),
    )

    forward_position_controller_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["forward_position_controller", "--controller-manager", "/controller_manager","--inactive"],
    )

    forward_controller_event_handler = RegisterEventHandler(
        OnProcessStart(
            target_action=robot_controller_spawner,
            on_start=[
                forward_position_controller_spawner
            ]
        )
    )

    switch_controllers = ExecuteProcess(
        condition=IfCondition(launch_servo),
        cmd=["ros2", "control", "switch_controllers", "--deactivate", robot_controller, "--activate", "forward_position_controller"],
        output="screen",
    )


    switch_controller_event_handler = RegisterEventHandler(
        OnProcessStart(
            target_action=forward_position_controller_spawner,
            on_start=[
                switch_controllers
            ]
        )
    )

    # Delay rviz start after `joint_state_broadcaster`
    delay_rviz_after_joint_state_broadcaster_spawner = RegisterEventHandler(
        event_handler=OnProcessExit(
            target_action=joint_state_broadcaster_spawner,
            on_exit=[rviz_node],
        )
    )

    # Delay start of robot_controller after `joint_state_broadcaster`
    delay_robot_controller_spawner_after_joint_state_broadcaster_spawner = RegisterEventHandler(
        event_handler=OnProcessExit(
            target_action=joint_state_broadcaster_spawner,
            on_exit=[robot_controller_spawner],
        )
    )

    # Gazebo related nodes
    gz_launch_description_with_gui = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            [FindPackageShare("ros_gz_sim"), "/launch/gz_sim.launch.py"]
        ),
        launch_arguments={"gz_args":"-r-v 4 empty.sdf"}.items(),
        condition=IfCondition(use_sim),
    )
    # Make topics available in ROS2
    pkg_project_bringup = get_package_share_directory('melfa_bringup')
    gz_sim_bridge = Node(
        package="ros_gz_bridge",
        executable="parameter_bridge",
        parameters=[{
            'config_file': os.path.join(pkg_project_bringup, 'config', 'ros_gz_bridge.yaml'),
            'qos_overrides./tf_static.publisher.durability': 'transient_local',
        }],
        output="screen",
        condition=IfCondition(use_sim),
    )

    gz_spawn_entity = Node(
        package="ros_gz_sim",
        executable="create",
        output="screen",
        arguments=[
            "-string",
            robot_description_content,
            "-allow_renaming",
            "true",
        ],
        condition=IfCondition(use_sim),
    )

    nodes = [
        control_node,
        robot_state_pub_node,
        joint_state_broadcaster_spawner,
        gpio_controller_spawner,
        forward_controller_event_handler,
        switch_controller_event_handler,
        delay_rviz_after_joint_state_broadcaster_spawner,
        delay_robot_controller_spawner_after_joint_state_broadcaster_spawner,
        gz_launch_description_with_gui,
        gz_sim_bridge,
        gz_spawn_entity,
    ]

    return LaunchDescription(declared_arguments + nodes)
