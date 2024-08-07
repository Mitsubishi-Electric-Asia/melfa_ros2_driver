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
  
#include "melfa_io_controllers/melfa_io_controllers.hpp"

namespace melfa_io_controllers
{
  controller_interface::CallbackReturn MelfaGPIOController::on_init()
  {
    /**
     * @brief on_init method for MelfaGPIOController class
     *
     * This method intiailizes the limits of the IO interfaces by reading from the IO limits YAML config file
     *
     * @returns CallbackReturn::SUCCESS if IO limits components are loading in YAML node
     * @returns CallbackReturn::ERROR if exception is raised
     *
     */

    try
    {
      const std::string package_name = "melfa_io_controllers";
      std::string confg_file = "io_limits.yaml";
      std::string yamlFilePath = ament_index_cpp::get_package_share_directory(package_name) + "/config/" + confg_file;

      // Load the YAML file
      io_limits_ = YAML::LoadFile(yamlFilePath);
    }
    catch (const std::exception &e)
    {
      fprintf(stderr, "Exception thrown during init stage with message: %s \n", e.what());
      return controller_interface::CallbackReturn::ERROR;
    }
    return controller_interface::CallbackReturn::SUCCESS;
  }

  controller_interface::InterfaceConfiguration MelfaGPIOController::command_interface_configuration() const
  {
    /**
     * @brief configuration method for MelfaGPIOController's command interface
     *
     * This method configures command interfaces for Melfa GPIO controller
     *
     * @return An InterfaceConfiguration object representing the configuration of the command interface
     *
     */
    controller_interface::InterfaceConfiguration config;
    config.type = controller_interface::interface_configuration_type::INDIVIDUAL;

    config.names.insert(config.names.end(), hand_io_command_interfaces_.begin(), hand_io_command_interfaces_.end());
    config.names.insert(config.names.end(), plc_link_io_command_interfaces_.begin(), plc_link_io_command_interfaces_.end());
    config.names.insert(config.names.end(), safety_io_command_interfaces_.begin(), safety_io_command_interfaces_.end());
    config.names.insert(config.names.end(), io_unit_command_interfaces_.begin(), io_unit_command_interfaces_.end());
    config.names.insert(config.names.end(), misc1_io_command_interfaces_.begin(), misc1_io_command_interfaces_.end());
    config.names.insert(config.names.end(), misc2_io_command_interfaces_.begin(), misc2_io_command_interfaces_.end());
    config.names.insert(config.names.end(), misc3_io_command_interfaces_.begin(), misc3_io_command_interfaces_.end());
    config.names.insert(config.names.end(), misc_io_command_interfaces_.begin(), misc_io_command_interfaces_.end());

    return config;
  }

  controller_interface::InterfaceConfiguration MelfaGPIOController::state_interface_configuration() const
  {
    /**
     * @brief configuration method for MelfaGPIOController's state interface
     *
     * This method configures state interfaces for Melfa GPIO controller
     *
     * @return An InterfaceConfiguration object representing the configuration of the state interface
     *
     */
    controller_interface::InterfaceConfiguration config;
    config.type = controller_interface::interface_configuration_type::INDIVIDUAL;

    config.names.insert(config.names.end(), hand_io_state_interfaces_.begin(), hand_io_state_interfaces_.end());
    config.names.insert(config.names.end(), plc_link_io_state_interfaces_.begin(), plc_link_io_state_interfaces_.end());
    config.names.insert(config.names.end(), safety_io_state_interfaces_.begin(), safety_io_state_interfaces_.end());
    config.names.insert(config.names.end(), io_unit_state_interfaces_.begin(), io_unit_state_interfaces_.end());
    config.names.insert(config.names.end(), misc1_io_state_interfaces_.begin(), misc1_io_state_interfaces_.end());
    config.names.insert(config.names.end(), misc2_io_state_interfaces_.begin(), misc2_io_state_interfaces_.end());
    config.names.insert(config.names.end(), misc3_io_state_interfaces_.begin(), misc3_io_state_interfaces_.end());
    config.names.insert(config.names.end(), misc_io_state_interfaces_.begin(), misc_io_state_interfaces_.end());

    return config;
  }

  controller_interface::return_type MelfaGPIOController::update(
      const rclcpp::Time & /*time*/, const rclcpp::Duration & /*period*/)
  {
    /**
     * @brief Update method for MelfaGPIOController class
     *
     * This method periodically updates IO state information, binary IO control mode and Controller Type
     *
     * @param time The time recorded at the beginning of the current iteration of the update loop
     * @param period The duration measured for the last iteration of the update loop
     * @returns Controller_interface::return_type::OK after update of the states
     * @note updateGPIO function faciliates updating different IO interfaces current state
     *
     */

    auto updateGPIO = [&](melfa_msgs::msg::GpioState &gpio_msg, const char *io_interface_name, GpioIdentifier identifier,
                          rclcpp::Publisher<melfa_msgs::msg::GpioState>::SharedPtr &gpio_msg_publisher_)
    {
      gpio_msg.interface_name = io_interface_name;
      gpio_msg.bitid = static_cast<uint16_t>(state_interfaces_.at(identifier).get_value());
      gpio_msg.bitmask = static_cast<uint16_t>(state_interfaces_.at(identifier + 1).get_value());
      gpio_msg.bit_send_type = (state_interfaces_.at(identifier + 2).get_value() == 0) ? "MXT_IO_NULL" : ((state_interfaces_.at(identifier + 2).get_value() == 1) ? "MXT_IO_OUT" : "MXT_IO_IN");
      gpio_msg.input_data = static_cast<uint16_t>(state_interfaces_.at(identifier + 3).get_value());
      gpio_msg.output_data = static_cast<uint16_t>(state_interfaces_.at(identifier + 4).get_value());
      gpio_msg_publisher_->publish(gpio_msg);
    };

    // Publishes updated IO State Interfaces
    updateGPIO(hand_gpio_msg_, "Hand IO State", GpioIdentifier::hand_io_, hand_io_state_publisher_);
    updateGPIO(plc_link_gpio_msg_, "PLC Link IO State", GpioIdentifier::plc_link_io_, plc_link_io_state_publisher_);
    updateGPIO(safety_gpio_msg_, "Safety IO State", GpioIdentifier::safety_io_, safety_io_state_publisher_);
    updateGPIO(io_unit_gpio_msg_, "IO Unit State", GpioIdentifier::io_unit_, io_unit_state_publisher_);
    updateGPIO(misc1_gpio_msg_, "Misc1 IO State", GpioIdentifier::misc1_io_, misc1_io_state_publisher_);
    updateGPIO(misc2_gpio_msg_, "Misc2 IO State", GpioIdentifier::misc2_io_, misc2_io_state_publisher_);
    updateGPIO(misc3_gpio_msg_, "Misc3 IO State", GpioIdentifier::misc3_io_, misc3_io_state_publisher_);



    // Publishes Binary IO Control Mode
    control_mode_config_ = std::bitset<7>(state_interfaces_.at(GpioIdentifier::control_mode_io).get_value()).to_ulong();

    io_control_mode.hand_io_interface = (control_mode_config_ & 0b0000001) != 0;
    io_control_mode.plc_link_io_interface = (control_mode_config_ & 0b0000010) != 0;
    io_control_mode.safety_io_interface = (control_mode_config_ & 0b0000100) != 0;
    io_control_mode.io_unit_interface = (control_mode_config_ & 0b0001000) != 0;
    io_control_mode.misc1_io_interface = (control_mode_config_ & 0b0010000) != 0;
    io_control_mode.misc2_io_interface = (control_mode_config_ & 0b0100000) != 0;
    io_control_mode.misc3_io_interface = (control_mode_config_ & 0b1000000) != 0;
    

    control_mode_publisher_->publish(io_control_mode);

    // Resets Command interfaces for disabled IO interface
    auto resetIOCommandInterface = [&](GpioIdentifier identifier, bool skipReset)
    {
      if (!skipReset)
      {
        for (int i = 0; i < 5; ++i)
          command_interfaces_[identifier + i].set_value(0.0);
      }
    };

    resetIOCommandInterface(GpioIdentifier::hand_io_, control_mode_config_ & 0b0000001);
    resetIOCommandInterface(GpioIdentifier::plc_link_io_, control_mode_config_ & 0b0000010);
    resetIOCommandInterface(GpioIdentifier::safety_io_, control_mode_config_ & 0b0000100);
    resetIOCommandInterface(GpioIdentifier::io_unit_, control_mode_config_ & 0b0001000);
    resetIOCommandInterface(GpioIdentifier::misc1_io_, control_mode_config_ & 0b0010000);
    resetIOCommandInterface(GpioIdentifier::misc2_io_, control_mode_config_ & 0b0100000);
    resetIOCommandInterface(GpioIdentifier::misc3_io_, control_mode_config_ & 0b1000000);




    ctrl_type_msg.controller_type = (state_interfaces_.at(GpioIdentifier::ctrl_type).get_value() == 1.0) ? "R" : ((state_interfaces_.at(GpioIdentifier::ctrl_type).get_value() == 2.0) ? "Q" : "D");

    controller_type_publisher_->publish(ctrl_type_msg);

    return controller_interface::return_type::OK;
  }

  controller_interface::CallbackReturn MelfaGPIOController::on_configure(
      const rclcpp_lifecycle::State & /*previous_state*/)
  {
    /**
     * @brief configuration method for MelfaGPIOController class
     *
     * This method retrieves IO state and command interfaces, creates publishers for IO interfaces state,
     * current binary IO control mode, controller type and services for configuring IO interfaces and IO control mode
     *
     * @param previous_state lifecycle state object representing state before current state
     * @returns CallbackReturn::SUCCESS if interfaces are retrieved, publishers and services are created
     * @returns CallbackReturn::ERROR if exception is raised
     */
    try
    {
      hand_io_state_interfaces_ = get_node()->get_parameter("hand_io_state_interfaces").as_string_array();
      plc_link_io_state_interfaces_ = get_node()->get_parameter("plc_link_io_state_interfaces").as_string_array();
      safety_io_state_interfaces_ = get_node()->get_parameter("safety_io_state_interfaces").as_string_array();
      io_unit_state_interfaces_ = get_node()->get_parameter("io_unit_state_interfaces").as_string_array();
      misc1_io_state_interfaces_ = get_node()->get_parameter("misc1_io_state_interfaces").as_string_array();
      misc2_io_state_interfaces_ = get_node()->get_parameter("misc2_io_state_interfaces").as_string_array();
      misc3_io_state_interfaces_ = get_node()->get_parameter("misc3_io_state_interfaces").as_string_array();
      misc_io_state_interfaces_ = get_node()->get_parameter("misc_io_state_interfaces").as_string_array();

      hand_io_command_interfaces_ = get_node()->get_parameter("hand_io_command_interfaces").as_string_array();
      plc_link_io_command_interfaces_ = get_node()->get_parameter("plc_link_io_command_interfaces").as_string_array();
      safety_io_command_interfaces_ = get_node()->get_parameter("safety_io_command_interfaces").as_string_array();
      io_unit_command_interfaces_ = get_node()->get_parameter("io_unit_command_interfaces").as_string_array();
      misc1_io_command_interfaces_ = get_node()->get_parameter("misc1_io_command_interfaces").as_string_array();
      misc2_io_command_interfaces_ = get_node()->get_parameter("misc2_io_command_interfaces").as_string_array();
      misc3_io_command_interfaces_ = get_node()->get_parameter("misc3_io_command_interfaces").as_string_array();

      misc_io_command_interfaces_ = get_node()->get_parameter("misc_io_command_interfaces").as_string_array();

      hand_io_state_publisher_ = get_node()->create_publisher<melfa_msgs::msg::GpioState>(
          "~/hand_io_state", rclcpp::SystemDefaultsQoS());

      plc_link_io_state_publisher_ = get_node()->create_publisher<melfa_msgs::msg::GpioState>(
          "~/plc_link_io_state", rclcpp::SystemDefaultsQoS());

      safety_io_state_publisher_ = get_node()->create_publisher<melfa_msgs::msg::GpioState>(
          "~/safety_io_state", rclcpp::SystemDefaultsQoS());

      io_unit_state_publisher_ = get_node()->create_publisher<melfa_msgs::msg::GpioState>(
          "~/io_unit_state", rclcpp::SystemDefaultsQoS());
      
      misc1_io_state_publisher_ = get_node()->create_publisher<melfa_msgs::msg::GpioState>(
          "~/misc1_io_state", rclcpp::SystemDefaultsQoS());

      misc2_io_state_publisher_ = get_node()->create_publisher<melfa_msgs::msg::GpioState>(
          "~/misc2_io_state", rclcpp::SystemDefaultsQoS());

      misc3_io_state_publisher_ = get_node()->create_publisher<melfa_msgs::msg::GpioState>(
          "~/misc3_io_state", rclcpp::SystemDefaultsQoS());

      controller_type_publisher_ = get_node()->create_publisher<melfa_msgs::msg::ControllerType>(
          "~/controller_type", rclcpp::SystemDefaultsQoS());

      control_mode_publisher_ = get_node()->create_publisher<melfa_msgs::msg::ControlMode>(
          "~/io_control_mode", rclcpp::SystemDefaultsQoS());

      gpio_command_subscriber_ = get_node()->create_subscription<melfa_msgs::msg::GpioCommand>(
          "~/gpio_command", rclcpp::SystemDefaultsQoS(),
          [this](const melfa_msgs::msg::GpioCommand::SharedPtr gpio_msg)
          { commandGPIOCallback(gpio_msg); });

      configure_gpio_srv_ = get_node()->create_service<melfa_msgs::srv::GpioConfigure>(
          "~/configure_gpio", std::bind(&MelfaGPIOController::configGpio, this, std::placeholders::_1, std::placeholders::_2));

      configure_mode_srv_ = get_node()->create_service<melfa_msgs::srv::ModeConfigure>(
          "~/configure_mode", std::bind(&MelfaGPIOController::configControlMode, this, std::placeholders::_1, std::placeholders::_2));
    }
    catch (...)
    {
      return LifecycleNodeInterface::CallbackReturn::ERROR;
    }
    return LifecycleNodeInterface::CallbackReturn::SUCCESS;
  }

  bool MelfaGPIOController::configGpio(melfa_msgs::srv::GpioConfigure::Request::SharedPtr req, melfa_msgs::srv::GpioConfigure::Response::SharedPtr res)
  {
    /**
     * @brief GPIO configuration method for MelfaGPIOController class
     *
     * This method binds to configure the GPIO service.
     *
     * @param req The request containing IO interface configuration parameters
     * @param res The response with the result of the configuration service
     * @returns res->success as true if configured else false
     */
    auto processIO = [&](auto &command_interfaces_, melfa_msgs::srv::GpioConfigure::Request::SharedPtr req, GpioIdentifier identifier,
                         const std::array<uint16_t, 3> &mode_values, long int ctrl_limit)
    {
      res->success = true;

      int limit_mask_ = (1 << std::min(16, static_cast<int>(ctrl_limit - req->bitid) + 1)) - 1;

      if (req->mode == req->SET_WRITE_OUT)
      {
        command_interfaces_[identifier + 1].set_value(static_cast<double>(req->bitmask & limit_mask_));
        command_interfaces_[identifier + 2].set_value(1.0);
        command_interfaces_[identifier + 3].set_value(1.0);
      }
      else if (req->mode == req->SET_READ_OUT)
      {
        command_interfaces_[identifier + 1].set_value(static_cast<double>(mode_values[1]));
        command_interfaces_[identifier + 2].set_value(1.0);
        command_interfaces_[identifier + 3].set_value(1.0);
      }
      else if (req->mode == req->SET_READ_IN)
      {
        command_interfaces_[identifier + 1].set_value(static_cast<double>(mode_values[2]));
        command_interfaces_[identifier + 2].set_value(2.0);
        command_interfaces_[identifier + 3].set_value(0.0);
      }
      else
      {
        res->success = false;
        RCLCPP_WARN(
            get_node()->get_logger(),
            "configure_gpio service: INVALID MODE REQUEST. "
            "Please select one of the following modes: WRITE_OUT, READ_OUT, READ_IN.");
        return;
      }

      command_interfaces_[identifier].set_value(static_cast<double>(req->bitid));
      command_interfaces_[identifier + 4].set_value(static_cast<double>(req->bitdata));
    };

    if (req->bitid >= ctrl_limits[0] && req->bitid <= ctrl_limits[1])
    {
      processIO(command_interfaces_, req, GpioIdentifier::hand_io_, hand_io_mode_, ctrl_limits[1]);
    }
    else if (req->bitid >= ctrl_limits[2] && req->bitid <= ctrl_limits[3])
    {
      processIO(command_interfaces_, req, GpioIdentifier::plc_link_io_, plc_link_io_mode_, ctrl_limits[3]);
    }
    else if ((req->mode == req->SET_READ_IN &&
              ((req->bitid >= ctrl_limits[4] && req->bitid <= ctrl_limits[5]) ||
               (req->bitid >= ctrl_limits[6] && req->bitid <= ctrl_limits[7]))) ||
             (req->mode == req->SET_READ_OUT &&
              ((req->bitid >= ctrl_limits[8] && req->bitid <= ctrl_limits[9]) ||
               (req->bitid >= ctrl_limits[10] && req->bitid <= ctrl_limits[11]))))
    {
      processIO(command_interfaces_, req, GpioIdentifier::safety_io_, safety_io_mode, ctrl_limits[7]);
    }
    else if (req->bitid >= ctrl_limits[12] && req->bitid <= ctrl_limits[13])
    {
      processIO(command_interfaces_, req, GpioIdentifier::io_unit_, io_unit_mode_, ctrl_limits[13]);
    }
    else if (req->bitid >= ctrl_limits[14] && req->bitid <= ctrl_limits[15])
    {
      processIO(command_interfaces_, req, GpioIdentifier::misc1_io_, misc1_io_mode_, ctrl_limits[15]);
    }
    else if (req->bitid >= ctrl_limits[16] && req->bitid <= ctrl_limits[17])
    {
      processIO(command_interfaces_, req, GpioIdentifier::misc2_io_, misc2_io_mode_, ctrl_limits[17]);
    }
    else if (req->bitid >= ctrl_limits[18] && req->bitid <= ctrl_limits[19])
    {
      processIO(command_interfaces_, req, GpioIdentifier::misc3_io_, misc3_io_mode_, ctrl_limits[19]);
    }
    else
    {
      RCLCPP_WARN(
          get_node()->get_logger(),
          "configure_gpio service: INVALID BIT TOP (%d) or INVALID MODE REQUEST. "
          "Please select Bit Top within acceptable range of IO interfaces (or) "
          "Please select one of the following modes: WRITE_OUT, READ_OUT, READ_IN.",
          req->bitid);
    }

    return res->success;
  }

  void MelfaGPIOController::commandGPIOCallback(const melfa_msgs::msg::GpioCommand::SharedPtr io_cmd_)
  {
    /**
     * @brief GPIO command callback method for MelfaGPIOController class
     *
     * This method binds to gpio_command_subscriber_
     *
     * @param io_cmd_ The message containing IO interface configuration parameters
     */

    auto commandGPIO = [&](GpioIdentifier identifier, long int ctrl_limit)
    {
      auto cmdType = [](const std::string &typeSendRecv) -> double
      {
        return (typeSendRecv == "MXT_IO_NULL") ? 0.0 : (typeSendRecv == "MXT_IO_OUT") ? 1.0
                                                   : (typeSendRecv == "MXT_IO_IN")    ? 2.0
                                                                                      : 3.0;
      };

      int limit_mask_ = (1 << std::min(16, static_cast<int>(ctrl_limit - io_cmd_->bitid) + 1)) - 1;

      double recvType = cmdType(io_cmd_->bit_recv_type);
      double sendType = cmdType(io_cmd_->bit_send_type);

      if (recvType != 3.0 && sendType != 3.0)
      {

        command_interfaces_[identifier].set_value(static_cast<double>(io_cmd_->bitid));
        command_interfaces_[identifier + 1].set_value(static_cast<double>(io_cmd_->bitmask & limit_mask_));
        command_interfaces_[identifier + 2].set_value(recvType);
        command_interfaces_[identifier + 3].set_value(sendType);
        command_interfaces_[identifier + 4].set_value(static_cast<double>(io_cmd_->bitdata));
      }
      else
      {
        RCLCPP_WARN(
            get_node()->get_logger(),
            "gpio_command publisher: INVALID SEND/RECEIVE REQUEST. "
            "Please select one of the following for receive and send type: "
            "[MXT_IO_IN, MXT_IO_NULL] to read input (or) "
            "[MXT_IO_OUT, MXT_IO_OUT] to read/write output");
      }
    };

    if (io_cmd_->bitid >= ctrl_limits[0] && io_cmd_->bitid <= ctrl_limits[1])
    {
      commandGPIO(GpioIdentifier::hand_io_, ctrl_limits[1]);
    }
    else if (io_cmd_->bitid >= ctrl_limits[2] && io_cmd_->bitid <= ctrl_limits[3])
    {
      commandGPIO(GpioIdentifier::plc_link_io_, ctrl_limits[3]);
    }
    else if (((io_cmd_->bit_recv_type == "MXT_IO_IN" && io_cmd_->bit_send_type == "MXT_IO_NULL") &&
              ((io_cmd_->bitid >= ctrl_limits[4] && io_cmd_->bitid <= ctrl_limits[5]) ||
               (io_cmd_->bitid >= ctrl_limits[6] && io_cmd_->bitid <= ctrl_limits[7]))) ||
             ((io_cmd_->bit_recv_type == "MXT_IO_OUT" && io_cmd_->bit_send_type == "MXT_IO_OUT") &&
              ((io_cmd_->bitid >= ctrl_limits[8] && io_cmd_->bitid <= ctrl_limits[9]) ||
               (io_cmd_->bitid >= ctrl_limits[10] && io_cmd_->bitid <= ctrl_limits[11]))))
    {
      commandGPIO(GpioIdentifier::safety_io_, ctrl_limits[7]);
    }
    else if (io_cmd_->bitid >= ctrl_limits[12] && io_cmd_->bitid <= ctrl_limits[13])
    {
      commandGPIO(GpioIdentifier::io_unit_, ctrl_limits[13]);
    }
    else if (io_cmd_->bitid >= ctrl_limits[14] && io_cmd_->bitid <= ctrl_limits[15])
    {
      commandGPIO(GpioIdentifier::misc1_io_, ctrl_limits[15]);
    }
    else if (io_cmd_->bitid >= ctrl_limits[16] && io_cmd_->bitid <= ctrl_limits[17])
    {
      commandGPIO(GpioIdentifier::misc2_io_, ctrl_limits[17]);
    }
    else if (io_cmd_->bitid >= ctrl_limits[18] && io_cmd_->bitid <= ctrl_limits[19])
    {
      commandGPIO(GpioIdentifier::misc3_io_, ctrl_limits[19]);
    }
    else
    {
      RCLCPP_WARN(
          get_node()->get_logger(),
          "gpio_command publisher: INVALID BIT TOP (%d) or INVALID SEND/RECEIVE TYPE. "
          "Please select one of the following for receive and send type: "
          "[MXT_IO_IN, MXT_IO_NULL] to read input (or) "
          "[MXT_IO_OUT, MXT_IO_OUT] to read/write output",
          io_cmd_->bitid);
    }
  }

  bool MelfaGPIOController::configControlMode(melfa_msgs::srv::ModeConfigure::Request::SharedPtr req,
                                              melfa_msgs::srv::ModeConfigure::Response::SharedPtr res)
  {
    /**
     * @brief Control mode configuration method for MelfaGPIOController class
     *
     * This is a binding method for configure_mode_srv_ service
     *
     * @param req The request containing control mode configuration
     * @param res The response with the result of the configuration service.
     * @returns true if configured else false
     */
    std::string control_mode_binary_ = std::to_string(req->misc3_io_interface) +
                                       std::to_string(req->misc2_io_interface) +
                                       std::to_string(req->misc1_io_interface) +
                                       std::to_string(req->io_unit_interface) +
                                       std::to_string(req->safety_io_interface) +
                                       std::to_string(req->plc_link_io_interface) +
                                       std::to_string(req->hand_io_interface);
                                       

    int control_mode_int_ = std::bitset<7>(control_mode_binary_).to_ulong();

    res->success = (control_mode_int_ <= 0b1111111) ? (command_interfaces_[GpioIdentifier::control_mode_io].set_value(
                                                       static_cast<double>(control_mode_int_)),
                                                   true)
                                                : false;

    return res->success;
  }

  controller_interface::CallbackReturn MelfaGPIOController::on_activate(
      const rclcpp_lifecycle::State & /*previous_state*/)
  {
    /**
     * @brief activation method for MelfaGPIOController class
     *
     * This method retrieves specific IO limits from the yaml node based on controller type
     *
     * @param previous_state lifecycle state object representing state before current state
     * @returns CallbackReturn::SUCCESS if limits are retrieved
     */
    if (state_interfaces_.at(GpioIdentifier::ctrl_type).get_value() == 1.0)
      ctrl_limits = io_limits_["R"].as<std::vector<long int>>();
    if (state_interfaces_.at(GpioIdentifier::ctrl_type).get_value() == 2.0)
      ctrl_limits = io_limits_["Q"].as<std::vector<long int>>();
    if (state_interfaces_.at(GpioIdentifier::ctrl_type).get_value() == 3.0)
      ctrl_limits = io_limits_["D"].as<std::vector<long int>>();

    return LifecycleNodeInterface::CallbackReturn::SUCCESS;
  }

  controller_interface::CallbackReturn MelfaGPIOController::on_deactivate(
      const rclcpp_lifecycle::State & /*previous_state*/)
  {
    /**
     * @brief deactivation method for MelfaGPIOController class
     *
     * This method resets all IO state publishers, configuration services
     *
     * @param previous_state lifecycle state object representing state before current state
     * @returns CallbackReturn::SUCCESS if resets are done
     * @returns CallbackReturn::ERROR if exception is raised
     */
    try
    {
      hand_io_state_publisher_.reset();
      plc_link_io_state_publisher_.reset();
      safety_io_state_publisher_.reset();
      io_unit_state_publisher_.reset();
      misc1_io_state_publisher_.reset();
      misc2_io_state_publisher_.reset();
      misc3_io_state_publisher_.reset();
      configure_gpio_srv_.reset();
      configure_mode_srv_.reset();
    }
    catch (...)
    {
      return LifecycleNodeInterface::CallbackReturn::ERROR;
    }
    return LifecycleNodeInterface::CallbackReturn::SUCCESS;
  }

} // namespace melfa_io_controllers

#include "pluginlib/class_list_macros.hpp"

PLUGINLIB_EXPORT_CLASS(melfa_io_controllers::MelfaGPIOController, controller_interface::ControllerInterface)