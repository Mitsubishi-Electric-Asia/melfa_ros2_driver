<img src="./figures/MELFA_t.png" width="400" height="98"> <img src="./figures/ROS-AP-logo.png" width="208" height="98">

# __Connecting to RT ToolBox3 Simulator__

This section will guide you to establish a UDP/IP connection between your ROS 2 computer and _RT ToolBox3 simulator_. It is highly recommended to test your ROS 2 applications on __RT ToolBox3 simulator__ before testing it on a real robot. This is helpful as __RT ToolBox3 simulator__ provides "near real" experience with 1-to-1 error conditions, velocity profiles and "close to actual" cycle times. _RT ToolBox3 simulator_ works with __Real Time Monitoring__<sup>1</sup> function which is useful for optimizing your ROS 2 application.

&#10146; <sup>1</sup> __Real Time Monitoring__ : found in [CR750/CR751 Series Controller, CR800 Series Controller Ethernet Function Instruction Manual](https://www.mitsubishielectric.com/fa/download/search.page?mode=manual&kisyu=/robot&q=CR750%2FCR751%20Series%20Controller%2C%20CR800%20Series%20Controller%20Ethernet%20Function%20Instruction%20Manual&sort=0&style=0&lang=2&category1=0&filter_discontinued=0&filter_bundled=0) from [Robot Industrial/Collaborative Robot MELFA Manual](https://www.mitsubishielectric.com/fa/download/search.page?mode=manual&kisyu=/robot). Provides real time data at control cycle intervals, 3.5ms for CR800-R/D and 7.11ms for CR800-Q.

Sections:

1. __Verifying Local IP Address on Windows__
2. __Verifying Local IP Address on Ubuntu 22.04LTS__
3. __ROS 2 Connection to Simulator__

## __1. Verify Local IP Address on Windows__

This section will guide you to find your local IP address on your Windows10 device.

1. Go to __control panel__ &rArr; __Network and Internet__ &rArr; __Network and Sharing Centre__ and select __Change adapter settings__. Your Network adapters will be displayed in a popup window.

</br>

<img src="./figures/local_ip.png" width="960" height="540">

</br>

2. Select your network adapter that you wish to connect to your ROS 2 computer. Select __Properties__ &rArr; __Internet Protocol Version 4 (TCP/IPv4)__. In the popup window, you can change your IP to your preferred IP address in your local network.

</br>

<img src="./figures/local_ip_final.png" width="960" height="540">

</br>

3. Select __Simulator__ to launch __RT ToolBox3 simulator__.

</br>

<img src="./figures/click_sim.png" width="960" height="540">

</br>

4. On the __Operation Panel__ (Green popup window for simulated robot), click on the __Select__ button and select the _robot Program_ you have created. Click __OK__. It is highly recommended to increase the filter value in the robot program to account jitter due to Windows not being real-time.
```
Servo On
Open "ENET: 192.168.0.100" As #1 'Insert your Linux Machine IP address in this line. Open stores the IP address into variable 1.
Mxt 1,1,200 'The first 1 refers to the IP address from the line above. 
           'The second 1 configures Mxt to expect Joint commands. 
           'The 200 refers to a 200ms low pass filter.
End 

```

</br>

<img src="./figures/sel_prg.png" width="960" height="540">

</br>

5. On the __Operation Panel__ (Green popup window for simulated robot), click on the __blue START__ button 

</br>
  <img src="./figures/sel_start.png" width="960" height="540">

</br>

## __2. Verifying Local IP address on Ubuntu 22.04LTS__

This section will guide you to find your local IP address on your Ubuntu 22.04LTS device. The process is similar on other Ubuntu versions.

1. Go to __Settings__ &rArr; __Network__ &rArr; __Wired (+)__ to create a new ethernet profile. Under __IPv4 Method__, select __Manual__ and input your preferred IP address for your Linux computer.
</br>

  <img src="./figures/linux_ip.png" width="1000" height="800"> 

</br>

2. From your Windows computer, ping your Linux computer. You may not be able to ping your Windows computer from your Linux computer due to Windows settings. If ping is unsuccessful, verify your Windows and Linux IP addresses and try again.

~~~
#ping <Linux local IP address>
ping 192.168.3.150
~~~

## __3. ROS 2 Connection to Simulator__

1. Assuming that ping is successful, you are now ready to connect your RT ToolBox3 simulator as if it is a _real_ robot. For the purpose of this tutorial, run the following command in the terminal. This command will launch the bringup launch file for RV7FRL robot using the CR800-R robot controller.

The command argument "packet_lost_log:=0" turns off the warning message for packet losses. It is highly recommended to NOT turn it off when connecting to a real robot. However, connecting to a simulation is fine.

~~~
ros2 launch melfa_bringup rv7frl_control.launch.py use_fake_hardware:=false controller_type:="R" robot_ip:=192.168.3.100 packet_lost_log:=0
~~~

 To launch MoveIt Servo use the following command instead

~~~
ros2 launch melfa_bringup rv7frl_control.launch.py use_fake_hardware:=false controller_type:="R" robot_ip:=192.168.3.100 packet_lost_log:=0 launch_servo:=true
~~~

</br>

  <img src="./figures/rv7frl_bringup.png" width="1000" height="600">

</br>
2. To launch MoveIt. MELFA ROS2 moveit_config packages are natively compatible with OMPL, Pilz industrial planner, CHOMP and MoveIt Servo.

```
ros2 launch melfa_rv7frl_moveit_config rv7frl_moveit.launch.py
```

</br>

  <img src="./figures/rv7frl_moveit.png" width="1000" height="600">

</br>

### Other guides:
- [Home page](./../README.md)
- [MELFA ROS2 user guide](./melfa_ros2_driver.md) : Usage and Installation of MELFA ROS2.
- [RT ToolBox3 Setup](./rt_toolbox3_setup.md) : Create your first RT ToolBox3 Project File for ROS 2.
- [RT ToolBox3 Simulator Setup](./rt_sim_setup.md) : Connect to RT ToolBox3 simulator as if it is a real robot.
- [RT ToolBox3 Real Robot Setup](./rt_real_setup.md): Connect to a MELFA robot.