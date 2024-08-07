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
  
#ifndef MELFA_DRIVER__HARDWARE_INTERFACE_HPP_
#define MELFA_DRIVER__HARDWARE_INTERFACE_HPP_

// Include headers for ROS 2 control hardware_interface
#include "hardware_interface/hardware_info.hpp"                // Declaration related to hardware information
#include "hardware_interface/system_interface.hpp"              // Declaration related to system interfaces
#include "hardware_interface/types/hardware_interface_type_values.hpp" // Declaration related to type values of hardware interfaces

// Include headers for MELFA
#include "melfa_driver/melfa_rt_exc.hpp"                     // Declaration related to MELFA real-time exceptions
#include "melfa_driver/rt_exc_def.hpp"                       // Definition of MELFA real-time exceptions
#include "melfa_driver/visibility_control.h"                 // Visibility control for MELFA

// Include headers for ROS 2
#include "rclcpp/rclcpp.hpp"                                    // ROS 2 C++ client library
#include "rclcpp/macros.hpp"                                    // Macros for ROS 2 C++
#include "rclcpp_lifecycle/node_interfaces/lifecycle_node_interface.hpp" // Lifecycle node interface for ROS 2
#include "rclcpp_lifecycle/state.hpp"                           // State definitions for ROS 2 lifecycle nodes
#include "geometry_msgs/msg/transform_stamped.hpp"              // Message type for stamped transforms in ROS 2

// Include headers for system libraries
#include <bitset>                                               // Provides the std::bitset class for managing bit fields
#include <sstream>

namespace melfa_driver
{
enum MELFAGpioConstants {
    io_fb_pack_size = 5,
    io_cmd_pack_size = 5,
    io_interface_state_size = 1,
    io_interface_command_size = 1,
    input_state_idx_ = 3,
    output_state_idx_ = 4

};

class MELFAPositionHardwareInterface : public hardware_interface::SystemInterface
{
public:
  RCLCPP_SHARED_PTR_DEFINITIONS(MELFAPositionHardwareInterface)

  MELFA_HARDWARE_PUBLIC
  hardware_interface::CallbackReturn on_init(const hardware_interface::HardwareInfo& system_info) override;

  MELFA_HARDWARE_PUBLIC
  std::vector<hardware_interface::StateInterface> export_state_interfaces() override;

  MELFA_HARDWARE_PUBLIC
  std::vector<hardware_interface::CommandInterface> export_command_interfaces() override;

  MELFA_HARDWARE_PUBLIC
  hardware_interface::CallbackReturn on_activate(const rclcpp_lifecycle::State & previous_state) override;

  MELFA_HARDWARE_PUBLIC
  hardware_interface::CallbackReturn on_deactivate(const rclcpp_lifecycle::State & previous_state) override;

  MELFA_HARDWARE_PUBLIC
  hardware_interface::return_type read(const rclcpp::Time& time, const rclcpp::Duration& period) final;

  MELFA_HARDWARE_PUBLIC
  hardware_interface::return_type write(const rclcpp::Time& time, const rclcpp::Duration& period) final;

  std::vector<uint16_t> readIOLimits(const std::string& input);

  void setIO(const std::vector<double>& io_commands, bool read_only);

private:
  // communication API
  std::unique_ptr<MelfaEthernet::rtexc> api_wrap_;
  uint8_t is_scara;
  uint8_t is_j7;
  uint8_t is_j8;
  uint8_t j7_linear;
  uint8_t j8_linear;

  bool execution_init_;
  unsigned int binary_config_;
  
  // Joint Interfaces
  std::vector<double> joint_position_commands_;
  std::vector<double> joint_position_states_;

  // Hand Interfaces
  std::vector<double> hand_io_commands_;
  std::vector<double> hand_io_states_;

  // PLC link Interfaces
  std::vector<double> plc_link_io_commands_;
  std::vector<double> plc_link_io_states_;

  //  Safety Interfaces
  std::vector<double> safety_io_states_;
  std::vector<double> safety_io_commands_;
  
  //  IO Unit Interfaces
  std::vector<double> io_unit_states_;
  std::vector<double> io_unit_commands_;

  //  Misc1 Interfaces
  std::vector<double> misc1_io_states_;
  std::vector<double> misc1_io_commands_;

  //  Misc2 Interfaces
  std::vector<double> misc2_io_states_;
  std::vector<double> misc2_io_commands_;

  //  Misc3 Interfaces
  std::vector<double> misc3_io_states_;
  std::vector<double> misc3_io_commands_;

  // Binary IO Control Mode Interfaces 
  std::vector<double> mode_io_command_;
  std::vector<double> mode_io_state_;  

  // Melfa Controller Type Interfaces
  std::vector<double> ctrl_type_io_command_;
  std::vector<double> ctrl_type_io_state_;  

  std::string io_control_mode_;
  std::string controller_type_;

  // IO Interface Limits
  std::vector<uint16_t> hand_io_limits_;
  std::vector<uint16_t> plc_link_io_limits_;
  std::vector<uint16_t> safety_input_limits_;
  std::vector<uint16_t> safety_output_limits_;
  std::vector<uint16_t> io_unit_limits_; 
  std::vector<uint16_t> misc1_io_limits_;
  std::vector<uint16_t> misc2_io_limits_; 
  std::vector<uint16_t> misc3_io_limits_; 


};

} // namespace melfa_driver
#endif  // MELFA_DRIVER__HARDWARE_INTERFACE_HPP_