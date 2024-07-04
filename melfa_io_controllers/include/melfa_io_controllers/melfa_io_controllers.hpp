#ifndef MELFA_FR_IO__CONTROLLER_HPP_
#define MELFA_FR_IO__CONTROLLER_HPP_

// Include headers for MELFA and ros2_control interfaces
#include "controller_interface/controller_interface.hpp"
#include "melfa_io_controllers/visibility_control.h"


// Melfa message service interfaces
#include "melfa_msgs/srv/gpio_configure.hpp"
#include "melfa_msgs/srv/mode_configure.hpp"

#include "melfa_msgs/msg/gpio_state.hpp"
#include "melfa_msgs/msg/gpio_command.hpp"
#include "melfa_msgs/msg/controller_type.hpp"
#include "melfa_msgs/msg/control_mode.hpp"
#include "std_msgs/msg/string.hpp"

// Include headers for system libraries
#include "yaml-cpp/yaml.h"
#include "ament_index_cpp/get_package_share_directory.hpp"
#include <bitset>

namespace melfa_io_controllers
{

enum GpioIdentifier
{
  hand_io_ = 0,
  plc_link_io_ = 5,
  safety_io_ = 10,
  control_mode_io = 15,
  ctrl_type = 16,
};

class MelfaGPIOController : public controller_interface::ControllerInterface
{
public:
  RCLCPP_SHARED_PTR_DEFINITIONS(MelfaGPIOController)

  MELFA_FR_IO_CONTROLLERS_PUBLIC
  controller_interface::InterfaceConfiguration command_interface_configuration() const override;

  MELFA_FR_IO_CONTROLLERS_PUBLIC
  controller_interface::InterfaceConfiguration state_interface_configuration() const override;

  MELFA_FR_IO_CONTROLLERS_PUBLIC
  controller_interface::return_type update(
    const rclcpp::Time & time, const rclcpp::Duration & period) override;

  MELFA_FR_IO_CONTROLLERS_PUBLIC
  CallbackReturn on_configure(const rclcpp_lifecycle::State & previous_state) override;

  MELFA_FR_IO_CONTROLLERS_PUBLIC
  CallbackReturn on_activate(const rclcpp_lifecycle::State & previous_state) override;

  MELFA_FR_IO_CONTROLLERS_PUBLIC
  CallbackReturn on_deactivate(const rclcpp_lifecycle::State & previous_state) override;

  MELFA_FR_IO_CONTROLLERS_PUBLIC
  CallbackReturn on_init() override;

  const std::array<uint16_t, 3> hand_io_mode_{0xFFFF, 0x0, 0X0};
  const std::array<uint16_t, 3> plc_link_io_mode_{0xFFFF, 0x0, 0X0};
  const std::array<uint16_t, 3> safety_io_mode{0x0, 0x0, 0X0};
  
private:
  std::vector<std::string> hand_io_state_interfaces_;
  std::vector<std::string> plc_link_io_state_interfaces_;
  std::vector<std::string> safety_io_state_interfaces_;
  std::vector<std::string> misc_io_state_interfaces_;

  std::vector<std::string> hand_io_command_interfaces_;
  std::vector<std::string> plc_link_io_command_interfaces_;
  std::vector<std::string> safety_io_command_interfaces_;
  std::vector<std::string> misc_io_command_interfaces_;

  YAML::Node io_limits_;

  bool configGpio(melfa_msgs::srv::GpioConfigure::Request::SharedPtr req, melfa_msgs::srv::GpioConfigure::Response::SharedPtr res);

  bool configControlMode(melfa_msgs::srv::ModeConfigure::Request::SharedPtr req, melfa_msgs::srv::ModeConfigure::Response::SharedPtr res);


  void commandGPIOCallback(const melfa_msgs::msg::GpioCommand::SharedPtr gpio_msg);

protected:
  // GPIO command message
  std::shared_ptr<melfa_msgs::msg::GpioCommand> io_cmd_;

  // IO interface state publishers
  std::shared_ptr<rclcpp::Publisher<melfa_msgs::msg::GpioState>> hand_io_state_publisher_;
  std::shared_ptr<rclcpp::Publisher<melfa_msgs::msg::GpioState>> plc_link_io_state_publisher_;
  std::shared_ptr<rclcpp::Publisher<melfa_msgs::msg::GpioState>> safety_io_state_publisher_;

  // Melfa Controller type publisher
  std::shared_ptr<rclcpp::Publisher<melfa_msgs::msg::ControllerType>> controller_type_publisher_;

  // Binary IO control mode publisher
  std::shared_ptr<rclcpp::Publisher<melfa_msgs::msg::ControlMode>> control_mode_publisher_;

  // IO interface state messages
  melfa_msgs::msg::GpioState hand_gpio_msg_; 
  melfa_msgs::msg::GpioState plc_link_gpio_msg_; 
  melfa_msgs::msg::GpioState safety_gpio_msg_;

  // Melfa Controller type message 
  melfa_msgs::msg::ControllerType ctrl_type_msg;
 
  // Melfa Binary IO control mode message
  melfa_msgs::msg::ControlMode io_control_mode;

  // GPIO command subscriber
  rclcpp::Subscription<melfa_msgs::msg::GpioCommand>::SharedPtr gpio_command_subscriber_;

  // GPIO configuration service 
  rclcpp::Service<melfa_msgs::srv::GpioConfigure>::SharedPtr configure_gpio_srv_;

  // GPIO binary control mode service
  rclcpp::Service<melfa_msgs::srv::ModeConfigure>::SharedPtr configure_mode_srv_;

  std::vector<long int> ctrl_limits;
  unsigned int control_mode_config_;

};
}  // namespace melfa_io_controllers

#endif  // MELFA_FR_IO__CONTROLLER_HPP_