<img src="./figures/MELFA_t.png" width="400" height="98"> <img src="./figures/ROS-AP-logo.png" width="208" height="98">

### Other guides:
- [Home page](./../README.md)
- [MELFA ROS2 user guide](./melfa_ros2_driver.md) : Usage and Installation of MELFA ROS2.
- [RT Toolbox3 Setup](./rt_toolbox3_setup.md) : Create your first RT Toolbox3 Project File for ROS2.
- [RT Toolbox3 Simulator Setup](./rt_sim_setup.md) : Connect to RT Toolbox3 simulator as if it is a real robot.
- [RT Toolbox3 Real Robot Setup](./rt_real_setup.md): Connect to a MELFA robot.
  
# Packages in the Repository
- melfa_bringup: provides launch files for robot bringup
- melfa_description: contains robot descriptions
- melfa_driver: provides driver/hardware interface for communication with MELFA robots
- melfa_io_controllers: provides ROS2 controllers for GPIO control
- melfa_moveit_config: provides example MoveIt config and launch files for MELFA robots
- melfa_msgs: provides ROS2 msgs for MELFA robots

# Build
1. Clone this repository
2. cd ${workspace}
3. Update package dependency sources
```
rosdep update
```
4. Install dependencies
```
rosdep install -r --from-paths . --ignore-src --rosdistro humble -y
```
5. Build
```
colcon build --cmake-args -DCMAKE_BUILD_TYPE=Release
```

# Launch

```
source install/setup.bash
```
1. View robot
```
ros2 launch melfa_description view_RV7FRL.launch.py 
```
2. Launch robot arm 

2.1 Simulation (the controller_type and robot_ip values do not affect simulation outcomes)
```
ros2 launch melfa_bringup RV7FRL_control.launch.py use_fake_hardware:=true controller_type:=<CONTROLLER TYPE> robot_ip:=<ROBOT IP>
```
2.2 Use real robot (the controller_type ["R" (or) "Q" (or) "D"] and robot_ip should be aligned with actual robot) 
```
ros2 launch melfa_bringup RV7FRL_control.launch.py use_fake_hardware:=false controller_type:=<CONTROLLER TYPE>  robot_ip:=<ROBOT IP>
```
Eg:
```
ros2 launch melfa_bringup RV7FRL_control.launch.py use_fake_hardware:=false controller_type:="R" robot_ip:=192.168.0.20
```
3. Launch MoveIt
```
ros2 launch melfa_rv7frl_moveit_config RV7FRL_moveit.launch.py
```

# Test with GPIO 

1. Launch real robot arm with controller_type R/ Q / D [Terminal 1]
```
ros2 launch melfa_bringup RV7FRL_control.launch.py use_fake_hardware:=false controller_type:=<CONTROLLER TYPE> robot_ip:=<ROBOT IP>
```

2. To view the enabled IO interfaces 
```
ros2 topic echo /gpio_controller/io_control_mode
```

3. To enable desired IO interfaces [Below command lets user to read/write hand_io_interface, plc_link_io_interface, safety_io_interface] [Terminal 2]
```
ros2 service call /gpio_controller/configure_mode melfa_msgs/srv/ModeConfigure "{hand_io_interface: true, plc_link_io_interface: true, safety_io_interface: true}"
```

4. To configure IO use GPIO configure service [Terminal 2]
```
# Dedicated IO for R and Q
int16 RQ_STOP = 10000
int16 RQ_START = 10001
int16 RQ_ERRRESET = 10009
int16 RQ_SRVON = 10010
int16 RQ_SRVOFF = 10011

# Bit Mask Modes
uint16 BIT_TOP_MODE = 0x0001
uint16 PACKET_MODE = 0xFFFF


# IO Configuration
string SET_READ_OUT = "READ_OUT" # READS entire 16 bit
string SET_WRITE_OUT = "WRITE_OUT" # WRITES only the masked bits in bitmask
string SET_READ_IN = "READ_IN" # READS entire 16 bit


# Fields Required
uint16 bitid
string mode
uint16 bitdata
uint16 bitmask
---
bool success
```

| IO Mode  | Description|
| ---      | ---      |
| READ_OUT | Reads Output Signal|
| READ_IN  | Reads Input Signal |
| WRITE_OUT| Writes Output Signal|


Example 4.1: Writing Hand output value to close Hand port 1: 
```
ros2 service call /gpio_controller/configure_gpio melfa_msgs/srv/GpioConfigure "{bitid: 900, mode: "WRITE_OUT", bitdata: 0b10, bitmask: 0b11}"
```

To observe changes in Hand IO interface [Terminal 3]
```
ros2 topic echo /gpio_controller/hand_io_state
```

Example 4.2 Writing Hand output value to close all Hand ports:
```
ros2 service call /gpio_controller/configure_gpio melfa_msgs/srv/GpioConfigure "{bitid: 900, mode: "WRITE_OUT", bitdata: 0b10101010, bitmask: 0xFFFF}"
```
To observe changes in Hand IO interface [Terminal 3]
```
ros2 topic echo /gpio_controller/hand_io_state
```

Example 4.3: Reading PLC link output value [Note: bitdata and bitmask doesn't affect "READ_OUT" and "READ IN" mode]
```
ros2 service call /gpio_controller/configure_gpio melfa_msgs/srv/GpioConfigure "{bitid: 10048, mode: "READ_OUT", bitdata: 0x0, bitmask: 0x0}"
```

To observe changes in PLC link IO interface [Terminal 4]
```
ros2 topic echo /gpio_controller/plc_link_io_state
```

Example 4.4: To configure using Topic

| IO Mode  | bit_recv_type| bit_send_type| bitmask|
| ---      | ---          |---           |---     |
| READ_OUT | MXT_IO_OUT   |MXT_IO_OUT    |0x0     |
| READ_IN  | MXT_IO_IN    |MXT_IO_NULL   |0x0     |
| WRITE_OUT| MXT_IO_OUT   |MXT_IO_OUT    |0xFFFF  |

```
ros2 topic pub /gpio_controller/gpio_command melfa_msgs/msg/GpioCommand '{bitid: 900, bitmask: 0xF00F, bit_recv_type: "MXT_IO_OUT", bit_send_type: "MXT_IO_OUT", bitdata: 0xFFFF}'

```

Example 4.5: To configure IO control mode [Below command lets user read or write only hand_io_interface and safety_io_interface and disables plc_link_io_interface]
```
ros2 service call /gpio_controller/configure_mode melfa_msgs/srv/ModeConfigure "{hand_io_interface: true, plc_link_io_interface: false, safety_io_interface: true}"

```
Note: The above command disables any read or write operation for the plc_link_io_interface. To enable again, repeat the service call with plc_link_io_interface as "true"


5. To run realtime servo control

Launch robot driver simulation or real robot [Terminal 1]
```
ros2 launch melfa_bringup RV7FRL_control.launch.py use_fake_hardware:=true controller_type:=<CONTROLLER TYPE> robot_ip:=<ROBOT IP> launch_servo:=true
```

Launch moveit config [Terminal 2]
```
ros2 launch melfa_rv7frl_moveit_config RV7FRL_moveit.launch.py
```

Launch Servo Keyboard Input [Terminal 3]
```
ros2 run melfa_rv7frl_moveit_config servo_keyboard_input
```