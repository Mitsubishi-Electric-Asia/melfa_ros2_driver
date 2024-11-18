//  COPYRIGHT (C) 2024 Mitsubishi Electric Corporation

//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at

//      http://www.apache.org/licenses/LICENSE-2.0

//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#include "melfa_driver/hardware_interface.hpp"

namespace melfa_driver
{

std::vector<uint16_t> MELFAPositionHardwareInterface::readIOLimits(const std::string& limit_string)
{
  /**
   * @brief Reads configured IO Limit parameters
   *
   * This function reads the IO limit strings from hardware parmater and generates it as a vector
   *
   * @param limit_string A string with IO limits seperated by delimiter (,)
   * @return The delimited values as a vector
   *
   * @note The function can delimit and vectorize any number of values
   */

  std::vector<uint16_t> result;
  std::istringstream ss(limit_string);
  std::string delimted_value;

  while (std::getline(ss, delimted_value, ','))
  {
    result.push_back(static_cast<uint16_t>(std::stoi(delimted_value)));
  }
  return result;
}

void MELFAPositionHardwareInterface::setIO(const std::vector<double>& io_commands, bool read_only = false)
{
  /**
   * @brief Sets IO paramters for the Melfa API
   *
   * This function sets required IO parameters for the command packet sent to the API
   *
   * @param io_commands A vector with IO command parameters
   * @param read_only A boolean parameter to differentiate read and write
   * @return void
   */

  api_wrap_->cmd_pack.IOBitTop = static_cast<uint16_t>(io_commands[0]);
  api_wrap_->cmd_pack.IORecvType = static_cast<uint16_t>(io_commands[2]);
  api_wrap_->cmd_pack.IOSendType = static_cast<uint16_t>(io_commands[3]);
  api_wrap_->cmd_pack.IOBitData = static_cast<uint16_t>(io_commands[4]);
  if (read_only)
  {
    api_wrap_->cmd_pack.IOBitMask = 0;
  }
  else
  {
    api_wrap_->cmd_pack.IOBitMask = static_cast<uint16_t>(io_commands[1]);
  }
}

hardware_interface::CallbackReturn
MELFAPositionHardwareInterface::on_init(const hardware_interface::HardwareInfo& system_info)
{
  /**
   * @brief Initialization method for MELFAPositionHardwareInterface class
   *
   * This function initializes IO data entities and validates hardware components
   *
   * @param system_info hardware_info structure with data from robot description file.
   * @returns CallbackReturn::SUCCESS if components and interfaces meets set expectations
   * @returns CallbackReturn::ERROR if component or interface doesn't meet set expectations
   *
   */

  if (hardware_interface::SystemInterface::on_init(system_info) != hardware_interface::CallbackReturn::SUCCESS)
  {
    return hardware_interface::CallbackReturn::ERROR;
  }

  info_ = system_info;
  execution_init_ = true;

  // Reading IO limits from harware info structure for specific IO interfaces
  hand_io_limits_ = readIOLimits(info_.hardware_parameters["hand_io_limits"]);
  plc_link_io_limits_ = readIOLimits(info_.hardware_parameters["plc_link_io_limits"]);
  safety_input_limits_ = readIOLimits(info_.hardware_parameters["safety_input_limits"]);
  safety_output_limits_ = readIOLimits(info_.hardware_parameters["safety_output_limits"]);
  io_unit_limits_ = readIOLimits(info_.hardware_parameters["io_unit_limits"]);
  misc1_io_limits_ = readIOLimits(info_.hardware_parameters["misc1_io_limits"]);
  misc2_io_limits_ = readIOLimits(info_.hardware_parameters["misc2_io_limits"]);
  misc3_io_limits_ = readIOLimits(info_.hardware_parameters["misc3_io_limits"]);

  // Reading user defined IO binary control mode and Melfa Controller type
  io_control_mode_ = info_.hardware_parameters["io_control_mode"];
  controller_type_ = info_.hardware_parameters["controller_type"];

  // Joint position commmands and states initiailization
  joint_position_commands_.resize(info_.joints.size(), std::numeric_limits<double>::quiet_NaN());
  joint_position_states_.resize(info_.joints.size(), std::numeric_limits<double>::quiet_NaN());

  // Joint state and command interface validation
  for (const hardware_interface::ComponentInfo& joint : info_.joints)
  {
    if (joint.command_interfaces.size() != 1)
    {
      RCLCPP_FATAL(rclcpp::get_logger("MELFAPositionHardwareInterface"),
                   "Joint '%s' has %zu command interfaces found. 1 expected.", joint.name.c_str(),
                   joint.command_interfaces.size());
      return hardware_interface::CallbackReturn::ERROR;
    }

    if (joint.command_interfaces[0].name != hardware_interface::HW_IF_POSITION)
    {
      RCLCPP_FATAL(rclcpp::get_logger("MELFAPositionHardwareInterface"),
                   "Joint '%s' have %s command interfaces found the command interface. '%s' expected.",
                   joint.name.c_str(), joint.command_interfaces[0].name.c_str(), hardware_interface::HW_IF_POSITION);
      return hardware_interface::CallbackReturn::ERROR;
    }

    if (joint.state_interfaces.size() != 1)
    {
      RCLCPP_FATAL(rclcpp::get_logger("MELFAPositionHardwareInterface"),
                   "Joint '%s' has %zu state interface. 1 expected.", joint.name.c_str(),
                   joint.state_interfaces.size());
      return hardware_interface::CallbackReturn::ERROR;
    }

    if (joint.state_interfaces[0].name != hardware_interface::HW_IF_POSITION)
    {
      RCLCPP_FATAL(rclcpp::get_logger("MELFAPositionHardwareInterface"),
                   "Joint '%s' have %s state interface as first state interface. '%s' expected.", joint.name.c_str(),
                   joint.state_interfaces[0].name.c_str(), hardware_interface::HW_IF_POSITION);
      return hardware_interface::CallbackReturn::ERROR;
    }
  }

  // GPIO components, state and command interfaces validation
  if (info_.gpios.size() != 9)
  {
    RCLCPP_FATAL(rclcpp::get_logger("MELFAPositionHardwareInterface"),
                 "MELFAPositionHardwareInterface has '%ld' GPIO components, '%d' expected.", info_.gpios.size(), 5);
    return hardware_interface::CallbackReturn::ERROR;
  }
  for (const hardware_interface::ComponentInfo& gpios : info_.gpios)
  {
    if (gpios.name == "io_control_mode" || gpios.name == "ctrl")
    {
      if (gpios.command_interfaces.size() != 1)
      {
        RCLCPP_FATAL(rclcpp::get_logger("MELFAPositionHardwareInterface"),
                     "GPIO component %s has '%ld' command interfaces, '%d' expected, as it has no write permission.",
                     gpios.name.c_str(), gpios.command_interfaces.size(), 1);
        return hardware_interface::CallbackReturn::ERROR;
      }
      if (gpios.state_interfaces.size() != 1)
      {
        RCLCPP_FATAL(rclcpp::get_logger("MELFAPositionHardwareInterface"),
                     "GPIO component %s has '%ld' state interfaces, '%d' expected.", gpios.name.c_str(),
                     gpios.state_interfaces.size(), 1);
        return hardware_interface::CallbackReturn::ERROR;
      }
    }
    else
    {
      if (gpios.command_interfaces.size() != 5)
      {
        RCLCPP_FATAL(rclcpp::get_logger("MELFAPositionHardwareInterface"),
                     "GPIO component %s has '%ld' command interfaces, '%d' expected, as it has no write permission.",
                     gpios.name.c_str(), gpios.command_interfaces.size(), 5);
        return hardware_interface::CallbackReturn::ERROR;
      }
      if (gpios.state_interfaces.size() != 5)
      {
        RCLCPP_FATAL(rclcpp::get_logger("MELFAPositionHardwareInterface"),
                     "GPIO component %s has '%ld' state interfaces, '%d' expected.", gpios.name.c_str(),
                     gpios.state_interfaces.size(), 5);
        return hardware_interface::CallbackReturn::ERROR;
      }
    }
  }
  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn
MELFAPositionHardwareInterface::on_activate(const rclcpp_lifecycle::State& previous_state)
{
  /**
   * @brief Activation method for MELFAPositionHardwareInterface class
   *
   * This function initializes control cycle, communication, joint position states and controller type
   *
   * @param previous_state lifecycle state object representing state before current state
   * @returns CallbackReturn::SUCCESS if components and interfaces meets set expectations
   *
   */
  RCLCPP_INFO(rclcpp::get_logger("MELFAPositionHardwareInterface"), "Starting ...please wait...");

  // Configure control cycle period
  float control_cycle_period;
  if (info_.hardware_parameters["controller_type"] == "R")
    control_cycle_period = 3.5F;
  if (info_.hardware_parameters["controller_type"] == "Q")
    control_cycle_period = 7.11F;
  if (info_.hardware_parameters["controller_type"] == "D")
    control_cycle_period = 3.5F;

  // Configure communication settings
  api_wrap_ = std::make_unique<MelfaEthernet::rtexc>(control_cycle_period);
  api_wrap_->robot_ip.dst_ip_address = info_.hardware_parameters["robot_ip"];
  api_wrap_->robot_ip.port = stoi(info_.hardware_parameters["robot_port"]);
  is_scara = stoi(info_.hardware_parameters["scara"]);
  is_j7 = stoi(info_.hardware_parameters["is_j7"]);
  j7_linear = stoi(info_.hardware_parameters["j7_linear"]);
  is_j8 = stoi(info_.hardware_parameters["is_j8"]);
  j8_linear = stoi(info_.hardware_parameters["j8_linear"]);
  api_wrap_->create_port();

  api_wrap_->cmd_pack.send_type = MXT_TYP_JOINT;          // set joint cmd type to joint.
  *(api_wrap_->cmd_pack.mon_dat) = MXT_TYP_FB_JOINT;      // set first feedback to joint encoder feedback.
  *(api_wrap_->cmd_pack.mon_dat + 1) = MXT_TYP_FB_POSE;   // set second feedback to pose feedback.
  *(api_wrap_->cmd_pack.mon_dat + 2) = MXT_TYP_FB_PULSE;  // set thrid feedback to pulseper second.
  *(api_wrap_->cmd_pack.mon_dat + 3) = MXT_TYP_FBKCUR;    // set forth feedback to % current.

  // API debug mode
  api_wrap_->RT_API_DEBUG_MODE = 0;

  // Initialize external control in robot controller.
  if (api_wrap_->WriteToRobot_init_() != 0)
  {
    RCLCPP_FATAL(rclcpp::get_logger("MELFAPositionHardwareInterface"), "ERROR: Enable to connect to robot.");
    return hardware_interface::CallbackReturn::ERROR;
  }

  RCLCPP_INFO(rclcpp::get_logger("MELFAPositionHardwareInterface"), "System successfully started!");

  // Reads joint position state from Melfa API feedback packet
  joint_position_states_[0] = api_wrap_->fb_pack.jnt_EFB.j1;
  joint_position_states_[1] = api_wrap_->fb_pack.jnt_EFB.j2;
  if (is_scara == 1)
  {
    api_wrap_->fb_pack.jnt_EFB.j3 /= 1000.0;
  }
  joint_position_states_[2] = api_wrap_->fb_pack.jnt_EFB.j3;
  joint_position_states_[3] = api_wrap_->fb_pack.jnt_EFB.j4;
  joint_position_states_[4] = api_wrap_->fb_pack.jnt_EFB.j5;
  joint_position_states_[5] = api_wrap_->fb_pack.jnt_EFB.j6;
  if (is_j7 == 1)
  {
    if (j7_linear == 1)
    {
      api_wrap_->fb_pack.jnt_EFB.j7 /= 1000.0;
    }
    joint_position_states_[6] = api_wrap_->fb_pack.jnt_EFB.j7;
  }
  if (is_j8 == 1)
  {
    if (j8_linear == 1)
    {
      api_wrap_->fb_pack.jnt_EFB.j8 /= 1000.0;
    }
    joint_position_states_[7] = api_wrap_->fb_pack.jnt_EFB.j8;
  }
  joint_position_commands_ = joint_position_states_;

  // Initialization of IO commands
  hand_io_commands_[0] = hand_io_limits_[0];
  hand_io_commands_[1] = hand_io_states_[1];
  hand_io_commands_[2] = MXT_IO_OUT;
  hand_io_commands_[3] = MXT_IO_NULL;
  hand_io_commands_[4] = hand_io_states_[4];

  plc_link_io_commands_[0] = plc_link_io_limits_[0];
  plc_link_io_commands_[1] = plc_link_io_states_[1];
  plc_link_io_commands_[2] = MXT_IO_OUT;
  plc_link_io_commands_[3] = MXT_IO_NULL;
  plc_link_io_commands_[4] = plc_link_io_states_[4];

  safety_io_commands_[0] = safety_input_limits_[0];
  safety_io_commands_[1] = safety_io_states_[1];
  safety_io_commands_[2] = MXT_IO_IN;
  safety_io_commands_[3] = MXT_IO_NULL;
  safety_io_commands_[4] = safety_io_states_[4];

  io_unit_commands_[0] = io_unit_limits_[0];
  io_unit_commands_[1] = io_unit_states_[1];
  io_unit_commands_[2] = MXT_IO_OUT;
  io_unit_commands_[3] = MXT_IO_NULL;
  io_unit_commands_[4] = io_unit_states_[4];

  misc1_io_commands_[0] = misc1_io_limits_[0];
  misc1_io_commands_[1] = misc1_io_states_[1];
  misc1_io_commands_[2] = MXT_IO_OUT;
  misc1_io_commands_[3] = MXT_IO_NULL;
  misc1_io_commands_[4] = misc1_io_states_[4];

  misc2_io_commands_[0] = misc2_io_limits_[0];
  misc2_io_commands_[1] = misc2_io_states_[1];
  misc2_io_commands_[2] = MXT_IO_OUT;
  misc2_io_commands_[3] = MXT_IO_NULL;
  misc2_io_commands_[4] = misc2_io_states_[4];

  misc3_io_commands_[0] = misc3_io_limits_[0];
  misc3_io_commands_[1] = misc3_io_states_[1];
  misc3_io_commands_[2] = MXT_IO_OUT;
  misc3_io_commands_[3] = MXT_IO_NULL;
  misc3_io_commands_[4] = misc3_io_states_[4];

  // Intialization of Controller type command
  if (controller_type_ == "R")
    ctrl_type_io_command_[0] = 1.0;
  if (controller_type_ == "Q")
    ctrl_type_io_command_[0] = 2.0;
  if (controller_type_ == "D")
    ctrl_type_io_command_[0] = 3.0;

  mode_io_command_[0] = std::bitset<7>(io_control_mode_).to_ulong();

  RCLCPP_INFO(rclcpp::get_logger("MELFAPositionHardwareInterface"),
              "on_activate: Joint position commands set to current joint position states");

  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn
MELFAPositionHardwareInterface::on_deactivate(const rclcpp_lifecycle::State& previous_state)
{
  /**
   * @brief Deactivation method for MELFAPositionHardwareInterface class
   *
   * This function sends stopping signal to the Melfa Controlller API
   *
   * @param previous_state lifecycle state object representing state before current state
   * @returns CallbackReturn::SUCCESS if deactivation condition is met
   * @returns CallbackReturn::ERROR if deactivation condition is not met
   *
   */

  // Sends End command to the Melfa controller API for terminating communication
  RCLCPP_INFO(rclcpp::get_logger("MELFAPositionHardwareInterface"), "Stopping ...please wait...");
  api_wrap_->cmd_pack.cmd_type = MXT_CMD_END;
  int is_deactivated = api_wrap_->WriteToRobot_CMD_();

  if (is_deactivated == 0)
  {
    RCLCPP_INFO(rclcpp::get_logger("MELFAPositionHardwareInterface"), "System successfully stopped!");
    return hardware_interface::CallbackReturn::SUCCESS;
  }
  else
  {
    RCLCPP_INFO(rclcpp::get_logger("MELFAPositionHardwareInterface"), "Not able to stop system!!!");
    return hardware_interface::CallbackReturn::ERROR;
  }
}

std::vector<hardware_interface::StateInterface> MELFAPositionHardwareInterface::export_state_interfaces()
{
  /**
   * @brief State Interfaces export method for MELFAPositionHardwareInterface class
   *
   * This function exports available state interfaces to the relevant ROS2 controllers
   *
   * @returns state interfaces as vector
   * @note addStateInterfaces function faciliates addition of different IO state interfaces
   *
   */

  std::vector<hardware_interface::StateInterface> state_interfaces_;

  // Add joint position states as reference to State interfaces
  for (uint i = 0; i < info_.joints.size(); i++)
  {
    state_interfaces_.emplace_back(hardware_interface::StateInterface(
        info_.joints[i].name, hardware_interface::HW_IF_POSITION, &joint_position_states_[i]));
  }

  hand_io_states_.resize(MELFAGpioConstants::io_fb_pack_size);
  plc_link_io_states_.resize(MELFAGpioConstants::io_fb_pack_size);
  safety_io_states_.resize(MELFAGpioConstants::io_fb_pack_size);
  io_unit_states_.resize(MELFAGpioConstants::io_fb_pack_size);
  misc1_io_states_.resize(MELFAGpioConstants::io_fb_pack_size);
  misc2_io_states_.resize(MELFAGpioConstants::io_fb_pack_size);
  misc3_io_states_.resize(MELFAGpioConstants::io_fb_pack_size);

  mode_io_state_.resize(MELFAGpioConstants::io_interface_state_size);
  ctrl_type_io_state_.resize(MELFAGpioConstants::io_interface_state_size);

  size_t counter_ = 0;

  // Add IO states as reference to State interfaces
  auto addStateInterfaces = [&](const std::string& interface_name, auto& interface_states) {
    counter_ = 0;
    for (size_t i = 0; i < info_.gpios.size(); ++i)
    {
      if (info_.gpios[i].name == interface_name)
      {
        for (auto state_if : info_.gpios.at(i).state_interfaces)
        {
          state_interfaces_.emplace_back(
              hardware_interface::StateInterface(info_.gpios.at(i).name, state_if.name, &interface_states[counter_++]));
          RCLCPP_INFO(rclcpp::get_logger("MELFAPositionHardwareInterface"), "Added State Interface:  %s/%s",
                      info_.gpios.at(i).name.c_str(), state_if.name.c_str());
        }
        break;
      }
    }
  };

  // Add IO interfaces, control mode and controller type as reference to State interfaces
  addStateInterfaces("hand_io", hand_io_states_);
  addStateInterfaces("plc_link_io", plc_link_io_states_);
  addStateInterfaces("safety_io", safety_io_states_);
  addStateInterfaces("io_unit", io_unit_states_);
  addStateInterfaces("misc1_io", misc1_io_states_);
  addStateInterfaces("misc2_io", misc2_io_states_);
  addStateInterfaces("misc3_io", misc3_io_states_);
  addStateInterfaces("io_control_mode", mode_io_state_);
  addStateInterfaces("ctrl", ctrl_type_io_state_);

  return state_interfaces_;
}

std::vector<hardware_interface::CommandInterface> MELFAPositionHardwareInterface::export_command_interfaces()
{
  /**
   * @brief Command Interfaces export method for MELFAPositionHardwareInterface class
   *
   * This function exports available command interfaces to the relevant ROS2 controllers
   *
   * @returns command interfaces as vector
   * @note addCommandInterfaces function faciliates addition of different IO command interfaces
   *
   */

  std::vector<hardware_interface::CommandInterface> command_interfaces_;

  // Add joint position commands as reference to Command interfaces
  for (size_t i = 0; i < info_.joints.size(); ++i)
  {
    command_interfaces_.emplace_back(hardware_interface::CommandInterface(
        info_.joints[i].name, hardware_interface::HW_IF_POSITION, &joint_position_commands_[i]));
  }

  hand_io_commands_.resize(MELFAGpioConstants::io_cmd_pack_size);
  plc_link_io_commands_.resize(MELFAGpioConstants::io_cmd_pack_size);
  safety_io_commands_.resize(MELFAGpioConstants::io_cmd_pack_size);
  io_unit_commands_.resize(MELFAGpioConstants::io_cmd_pack_size);
  misc1_io_commands_.resize(MELFAGpioConstants::io_cmd_pack_size);
  misc2_io_commands_.resize(MELFAGpioConstants::io_cmd_pack_size);
  misc3_io_commands_.resize(MELFAGpioConstants::io_cmd_pack_size);
  mode_io_command_.resize(MELFAGpioConstants::io_interface_command_size);
  ctrl_type_io_command_.resize(MELFAGpioConstants::io_interface_command_size);

  size_t counter_ = 0;

  // Add IO commands as reference to Command interfaces
  auto addCommandInterfaces = [&](const std::string& interface_name, auto& interface_commands) {
    counter_ = 0;
    for (size_t i = 0; i < info_.gpios.size(); ++i)
    {
      if (info_.gpios[i].name == interface_name)
      {
        for (auto command_if : info_.gpios.at(i).command_interfaces)
        {
          command_interfaces_.emplace_back(hardware_interface::CommandInterface(info_.gpios.at(i).name, command_if.name,
                                                                                &interface_commands[counter_++]));
          RCLCPP_INFO(rclcpp::get_logger("MELFAPositionHardwareInterface"), "Added Command Interface:  %s/%s",
                      info_.gpios.at(i).name.c_str(), command_if.name.c_str());
        }
        break;
      }
    }
  };

  // Add IO interfaces, control mode and controller type as reference to Command interfaces
  addCommandInterfaces("hand_io", hand_io_commands_);
  addCommandInterfaces("plc_link_io", plc_link_io_commands_);
  addCommandInterfaces("safety_io", safety_io_commands_);
  addCommandInterfaces("io_unit", io_unit_commands_);
  addCommandInterfaces("misc1_io", misc1_io_commands_);
  addCommandInterfaces("misc2_io", misc2_io_commands_);
  addCommandInterfaces("misc3_io", misc3_io_commands_);
  addCommandInterfaces("io_control_mode", mode_io_command_);
  addCommandInterfaces("ctrl", ctrl_type_io_command_);

  return command_interfaces_;
}

hardware_interface::return_type MELFAPositionHardwareInterface::read(const rclcpp::Time& time,
                                                                     const rclcpp::Duration& period)
{
  /**
   * @brief Read method for MELFAPositionHardwareInterface class
   *
   * This function reads joint states and IO states from Melfa Controller API feedback
   *
   * @param time The time recorded at the beginning of the current iteration of the control loop
   * @param period The duration measured for the last iteration of the control loop.
   * @returns hardware_interface::return_type::OK after interfaces are read
   * @note readIOFeedback function faciliates reading different IO state interfaces
   *
   */

  // Read Feedback packet from Melfa Controller API
  if (api_wrap_->ReadFromRobot_FB_() != 0)
  {
    if (!api_wrap_->robot_status)
    {
      RCLCPP_FATAL(rclcpp::get_logger("MELFAPositionHardwareInterface"), "ERROR: Connection lost.");
      return hardware_interface::return_type::ERROR;
    }
    RCLCPP_WARN(rclcpp::get_logger("MELFAPositionHardwareInterface"), "WARN: Packet lost. %d",
                api_wrap_->packet_recv_lost);
    return hardware_interface::return_type::OK;
  }

  joint_position_states_[0] = api_wrap_->fb_pack.jnt_EFB.j1;
  joint_position_states_[1] = api_wrap_->fb_pack.jnt_EFB.j2;
  if (is_scara == 1)
  {
    api_wrap_->fb_pack.jnt_EFB.j3 /= 1000.0;
  }
  joint_position_states_[2] = api_wrap_->fb_pack.jnt_EFB.j3;
  joint_position_states_[3] = api_wrap_->fb_pack.jnt_EFB.j4;
  joint_position_states_[4] = api_wrap_->fb_pack.jnt_EFB.j5;
  joint_position_states_[5] = api_wrap_->fb_pack.jnt_EFB.j6;
  if (is_j7 == 1)
  {
    if (j7_linear == 1)
    {
      api_wrap_->fb_pack.jnt_EFB.j7 /= 1000.0;
    }
    joint_position_states_[6] = api_wrap_->fb_pack.jnt_EFB.j7;
  }
  if (is_j8 == 1)
  {
    if (j8_linear == 1)
    {
      api_wrap_->fb_pack.jnt_EFB.j8 /= 1000.0;
    }
    joint_position_states_[7] = api_wrap_->fb_pack.jnt_EFB.j8;
  }

  // Function to read IO feedbacks for different IO interfaces
  auto readIOFeedback = [&](std::vector<double>& io_states, int io_index,
                            std::unique_ptr<MelfaEthernet::rtexc>& api_wrap_) {
    io_states[0] = static_cast<double>(api_wrap_->fb_pack.IOBitTop);
    io_states[1] = static_cast<double>(api_wrap_->fb_pack.IOBitMask);
    io_states[2] = static_cast<double>(api_wrap_->fb_pack.IOSendType);
    io_states[io_index] = static_cast<double>(api_wrap_->fb_pack.IOBitData);
  };

  // Binary IO control mode and Controller Type state update
  mode_io_state_[0] = mode_io_command_[0];
  ctrl_type_io_state_[0] = ctrl_type_io_command_[0];

  // IO interface allocation based on IO bit top in the feedback
  if ((api_wrap_->fb_pack.IOBitTop >= hand_io_limits_[0]) && (api_wrap_->fb_pack.IOBitTop <= hand_io_limits_[1]))
  {
    readIOFeedback(hand_io_states_,
                   (api_wrap_->fb_pack.IOSendType == MXT_IO_IN) ? MELFAGpioConstants::input_state_idx_ :
                                                                  MELFAGpioConstants::output_state_idx_,
                   api_wrap_);
  }

  if ((api_wrap_->fb_pack.IOBitTop >= plc_link_io_limits_[0]) &&
      (api_wrap_->fb_pack.IOBitTop <= plc_link_io_limits_[1]))
  {
    readIOFeedback(plc_link_io_states_,
                   (api_wrap_->fb_pack.IOSendType == MXT_IO_IN) ? MELFAGpioConstants::input_state_idx_ :
                                                                  MELFAGpioConstants::output_state_idx_,
                   api_wrap_);
  }
  if ((((api_wrap_->fb_pack.IOBitTop >= safety_input_limits_[0]) &&
        (api_wrap_->fb_pack.IOBitTop <= safety_input_limits_[1])) ||
       ((api_wrap_->fb_pack.IOBitTop >= safety_input_limits_[2]) &&
        (api_wrap_->fb_pack.IOBitTop <= safety_input_limits_[3]))) &&
      (api_wrap_->fb_pack.IOSendType == MXT_IO_IN))
  {
    readIOFeedback(safety_io_states_, MELFAGpioConstants::input_state_idx_, api_wrap_);
  }

  else if ((((api_wrap_->fb_pack.IOBitTop >= safety_output_limits_[0]) &&
             (api_wrap_->fb_pack.IOBitTop <= safety_output_limits_[1])) ||
            ((api_wrap_->fb_pack.IOBitTop >= safety_output_limits_[2]) &&
             (api_wrap_->fb_pack.IOBitTop <= safety_output_limits_[3]))) &&
           (api_wrap_->fb_pack.IOSendType == MXT_IO_OUT))
  {
    readIOFeedback(safety_io_states_, MELFAGpioConstants::output_state_idx_, api_wrap_);
  }
  if ((api_wrap_->fb_pack.IOBitTop >= io_unit_limits_[0]) && (api_wrap_->fb_pack.IOBitTop <= io_unit_limits_[1]))
  {
    readIOFeedback(io_unit_states_,
                   (api_wrap_->fb_pack.IOSendType == MXT_IO_IN) ? MELFAGpioConstants::input_state_idx_ :
                                                                  MELFAGpioConstants::output_state_idx_,
                   api_wrap_);
  }
  if ((api_wrap_->fb_pack.IOBitTop >= misc1_io_limits_[0]) && (api_wrap_->fb_pack.IOBitTop <= misc1_io_limits_[1]))
  {
    readIOFeedback(misc1_io_states_,
                   (api_wrap_->fb_pack.IOSendType == MXT_IO_IN) ? MELFAGpioConstants::input_state_idx_ :
                                                                  MELFAGpioConstants::output_state_idx_,
                   api_wrap_);
  }
  if ((api_wrap_->fb_pack.IOBitTop >= misc2_io_limits_[0]) && (api_wrap_->fb_pack.IOBitTop <= misc2_io_limits_[1]))
  {
    readIOFeedback(misc2_io_states_,
                   (api_wrap_->fb_pack.IOSendType == MXT_IO_IN) ? MELFAGpioConstants::input_state_idx_ :
                                                                  MELFAGpioConstants::output_state_idx_,
                   api_wrap_);
  }
  if ((api_wrap_->fb_pack.IOBitTop >= misc3_io_limits_[0]) && (api_wrap_->fb_pack.IOBitTop <= misc3_io_limits_[1]))
  {
    readIOFeedback(misc3_io_states_,
                   (api_wrap_->fb_pack.IOSendType == MXT_IO_IN) ? MELFAGpioConstants::input_state_idx_ :
                                                                  MELFAGpioConstants::output_state_idx_,
                   api_wrap_);
  }
  // Resets State interfaces for disabled IO interface
  unsigned int control_mode_config_ = static_cast<unsigned int>(mode_io_state_[0]);

  auto resetIOFeedback = [&](std::vector<double>& io_states) { std::fill(io_states.begin(), io_states.end(), 0.0); };

  if ((control_mode_config_ & 0b0000001) == 0)
    resetIOFeedback(hand_io_states_);
  if ((control_mode_config_ & 0b0000010) == 0)
    resetIOFeedback(plc_link_io_states_);
  if ((control_mode_config_ & 0b0000100) == 0)
    resetIOFeedback(safety_io_states_);
  if ((control_mode_config_ & 0b0001000) == 0)
    resetIOFeedback(io_unit_states_);
  if ((control_mode_config_ & 0b0010000) == 0)
    resetIOFeedback(misc1_io_states_);
  if ((control_mode_config_ & 0b0100000) == 0)
    resetIOFeedback(misc2_io_states_);
  if ((control_mode_config_ & 0b1000000) == 0)
    resetIOFeedback(misc3_io_states_);

  return hardware_interface::return_type::OK;
}

hardware_interface::return_type MELFAPositionHardwareInterface::write(const rclcpp::Time& time,
                                                                      const rclcpp::Duration& period)
{
  /**
   * @brief Write method for MELFAPositionHardwareInterface class
   *
   * This function writes joint commands and IO commands to Melfa Controller API
   *
   * @param time The time recorded at the beginning of the current iteration of the control loop
   * @param period The duration measured for the last iteration of the control loop.
   * @returns hardware_interface::return_type::OK after writing interfaces to API.
   *
   */

  // Joint Position commands update from ROS2 controller to Melfa Controller API
  api_wrap_->cmd_pack.cmd_type = MXT_CMD_MOVE;

  api_wrap_->cmd_pack.jnt_CMD.j1 = joint_position_commands_[0];
  api_wrap_->cmd_pack.jnt_CMD.j2 = joint_position_commands_[1];
  api_wrap_->cmd_pack.jnt_CMD.j3 = joint_position_commands_[2];
  if (is_scara == 1)
  {
    api_wrap_->cmd_pack.jnt_CMD.j3 *= 1000.0;
  }
  api_wrap_->cmd_pack.jnt_CMD.j4 = joint_position_commands_[3];
  api_wrap_->cmd_pack.jnt_CMD.j5 = joint_position_commands_[4];
  api_wrap_->cmd_pack.jnt_CMD.j6 = joint_position_commands_[5];
  if (is_j7 == 1)
  {
    api_wrap_->cmd_pack.jnt_CMD.j7 = joint_position_commands_[6];
    if (j7_linear == 1)
    {
      api_wrap_->cmd_pack.jnt_CMD.j7 *= 1000.0;
    }
  }
  if (is_j8 == 1)
  {
    api_wrap_->cmd_pack.jnt_CMD.j8 = joint_position_commands_[7];
    if (j8_linear == 1)
    {
      api_wrap_->cmd_pack.jnt_CMD.j8 *= 1000.0;
    }
  }
  // Interface selection based on Binary IO control mode
  if (execution_init_)
  {
    binary_config_ = static_cast<unsigned int>(mode_io_command_[0]);
    execution_init_ = false;
  }

  if ((binary_config_ & 0b0000001) != 0)
  {
    setIO(hand_io_commands_);
    binary_config_ ^= 0b0000001;
  }
  else if ((binary_config_ & 0b0000010) != 0)
  {
    setIO(plc_link_io_commands_);
    binary_config_ ^= 0b0000010;
  }
  else if ((binary_config_ & 0b0000100) != 0)
  {
    setIO(safety_io_commands_, true);
    binary_config_ ^= 0b0000100;
  }
  else if ((binary_config_ & 0b0001000) != 0)
  {
    setIO(io_unit_commands_);
    binary_config_ ^= 0b0001000;
  }
  else if ((binary_config_ & 0b0010000) != 0)
  {
    setIO(misc1_io_commands_);
    binary_config_ ^= 0b0010000;
  }
  else if ((binary_config_ & 0b0100000) != 0)
  {
    setIO(misc2_io_commands_);
    binary_config_ ^= 0b0100000;
  }
  else if ((binary_config_ & 0b1000000) != 0)
  {
    setIO(misc3_io_commands_);
    binary_config_ ^= 0b1000000;
  }

  if (binary_config_ == 0)
  {
    execution_init_ = true;
  }

  if (api_wrap_->robot_status)
  {
    if (api_wrap_->WriteToRobot_CMD_() != 0)
    {
      if (!api_wrap_->robot_status)
      {
        RCLCPP_FATAL(rclcpp::get_logger("MELFAPositionHardwareInterface"), "ERROR: Connection lost.");
        return hardware_interface::return_type::ERROR;
      }
      RCLCPP_WARN(rclcpp::get_logger("MELFAPositionHardwareInterface"), "ERROR: Command Fail.");
      return hardware_interface::return_type::OK;
    }
      return hardware_interface::return_type::OK;
  }
  else
  {
    RCLCPP_FATAL(rclcpp::get_logger("MELFAPositionHardwareInterface"), "ERROR: Connection lost.");
    return hardware_interface::return_type::ERROR;
  }
}
}  // namespace melfa_driver

#include "pluginlib/class_list_macros.hpp"

PLUGINLIB_EXPORT_CLASS(melfa_driver::MELFAPositionHardwareInterface, hardware_interface::SystemInterface)