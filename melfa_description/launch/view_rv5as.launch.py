
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, RegisterEventHandler
from launch.conditions import IfCondition
from launch.event_handlers import OnProcessExit
from launch.substitutions import Command, FindExecutable, LaunchConfiguration, PathJoinSubstitution

from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    # Declare arguments
    declared_arguments = []
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
            default_value='RV-5AS/RV-5AS.urdf.xacro',
            description='URDF/XACRO description file with the robot.',
        )
    )
    declared_arguments.append(
        DeclareLaunchArgument(
            'prefix',
            default_value='""',
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
            'robot_ip',
            default_value='192.168.0.20',
            description='Robot IP.',
        )
    )
    declared_arguments.append(
        DeclareLaunchArgument(
            'robot_port',
            default_value='10100',
            description='Robot port.',
        )
    )
    declared_arguments.append(
        DeclareLaunchArgument(
            'initial_positions_file',
            default_value='initial_positions_rv.yaml',
            description='Configuration file of robot initial positions for simulation.',
        )
    )

    # Initialize Arguments
    description_package = LaunchConfiguration('description_package')
    description_file = LaunchConfiguration('description_file')
    prefix = LaunchConfiguration('prefix')
    use_sim = LaunchConfiguration('use_sim')
    use_fake_hardware = LaunchConfiguration('use_fake_hardware')
    robot_ip = LaunchConfiguration('robot_ip')
    robot_port = LaunchConfiguration('robot_port')
    initial_positions_file = LaunchConfiguration('initial_positions_file')
    
    initial_positions_file = PathJoinSubstitution(
        [FindPackageShare(description_package), "config", initial_positions_file]
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
        ]
    )
    robot_description = {'robot_description': robot_description_content}

    rviz_config_file = PathJoinSubstitution(
        [FindPackageShare(description_package), 'rviz', 'MEAP_RV-5AS.rviz']
    )

    joint_state_pub_gui_node = Node(
                                package='joint_state_publisher_gui',
                                executable='joint_state_publisher_gui',
                                name='joint_state_publisher_gui',
                                parameters=[robot_description])

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
    )
    nodes = [
        joint_state_pub_gui_node,
        robot_state_pub_node,
        rviz_node,
    ]

    return LaunchDescription(declared_arguments + nodes)
