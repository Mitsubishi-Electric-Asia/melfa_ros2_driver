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
  
/**
 * @file melfa_rt_exc.hpp
 * @author Liu Muyao
 * @brief Header file for Real Time External Control API
 *
 * @copyright Apache License 2.0
 *
 */

#ifndef __RTEXC__
#define __RTEXC__

#include "rt_exc_def.hpp"
#include <iostream>
#include <iomanip>
#include <string.h>
#ifdef _WIN32
#define _USE_MATH_DEFINES
#include <math.h>
#include <winsock2.h>
#include <windows.h>
#include <conio.h>
#include <ws2tcpip.h>
#include <ws2spi.h>
#endif
#ifdef __linux__
#define _USE_MATH_DEFINES
#include <cmath>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sched.h>
#include <sys/mman.h>
#endif
#define MAXBUFLEN 512  // Max Buffer Length
#define NO_FLAGS_SET 0 // Literally no flags set

/**
 * @brief Melfa Ethernet SDK namespace. Refer to Mitsubishi Electric Industrial Robot Ethernet Function Instruction Manual for detailed descriptions
 *
 */
namespace MelfaEthernet
{
    /**
     * @brief Real Time External Control Class Object. Refer to Mitsubishi Electric Industrial Robot Ethernet Function Instruction Manual for detailed descriptions
     *
     */
    class rtexc
    {
        protected:
        #if _WIN32
            WSAData Data;
        #endif
        /**
         * @brief Private variable. Receive transmission timeout setting. Operation Control Time: CR800 3.5ms. CR750/1 7.11ms.
         *
         */
        timeval sTimeOut;
        /**
         * @brief Private variable to store data types for monitoring
         *
         */
        int type_mon[4]; // Monitor data type.
        /**
         * @brief Packet variable for sending
         *
         */
        MXTCMD MXTsend;
        /**
         * @brief Packet variable for receiving
         *
         */
        MXTCMD MXTrecv;
        /**
         * @brief Private variable for console prints for debug mode
         *
         */
        char console_msg[MAXBUFLEN];
        /**
         * @brief Send array
         *
         */
        char sendText[MAXBUFLEN];
        /**
         * @brief Receive array
         *
         */
        char recvText[MAXBUFLEN];
        /**
         * @brief Command Structure used by Real Time External Control API
         *
         */
        typedef struct
        {
            /**
             * @brief Command. MXT_CMD_NULL for NULL command. MXT_CMD_MOVE for Move command.
             * @brief MXT_CMD_END to end real time external control function in robot controller
             */
            uint16_t cmd_type; // Command type 0/1/255
            /**
             * @brief Command data type designation. MXT_TYP_POSE for Pose. MXT_TYP_JOINT for joint. MXT_TYP_PULSE for pulse
             */
            uint16_t send_type; // Send data type
            /**
             * @brief data for Pose command
             *
             */
            POSE pos_CMD;
            /**
             * @brief data for Joint command
             *
             */
            JOINT jnt_CMD;
            /**
             * @brief data for Pulse command
             *
             */
            PULSE pls_CMD; // PULSE command 3
            /**
             * @brief Designate data types for monitoring
             *
             */
            uint16_t mon_dat[4];
            /**
             * @brief Send input/output signal data designation. MXT_IO_OUT for output signal. MXT_IO_NULL for input signal
             *
             */
            uint16_t IOSendType;
            /**
             * @brief Return input/output signal data designation. MXT_IO_OUT for output signal. MXT_IO_NULL for input signal
             *
             */
            uint16_t IORecvType;
            /**
             * @brief Head bit Number
             *
             */
            uint16_t IOBitTop;
            /**
             * @brief Transmission bit mask pattern designation (0x0000-0xffff)
             *
             */
            uint16_t IOBitMask;
            /**
             * @brief Input/output signal data (0x0000-0xffff)
             *
             */
            uint16_t IOBitData; // Variable for Input/output signal data (0x0000-0xffff), IoData

        } CMD_packet;
        /**
         * @brief Feedback structure used by Real Time External Control API
         *
         */
        typedef struct
        {
            /**
             * @brief POSE command data feedback. Data from MXT_TYP_POSE
             *
             */
            POSE pos_FB;
            /**
             * @brief JOINT command data feedback. Data from MXT_TYP_JOINT
             *
             */
            JOINT jnt_FB;
            /**
             * @brief PULSE command data feedback. Data from MXT_TYP_PULSE
             *
             */
            PULSE pls_FB;
            /**
             * @brief POSE filtered command data feedback. Data from MXT_TYP_FPOSE
             *
             */
            POSE pos_FFB;
            /**
             * @brief JOINT filtered command data feedback. Data from MXT_TYP_FJOINT
             *
             */
            JOINT jnt_FFB;
            /**
             * @brief PULSE filtered command data feedback. Data from MXT_TYP_FPULSE
             *
             */
            PULSE pls_FFB;
            /**
             * @brief POSE Encoder feedback. Data from MXT_TYP_FB_POSE
             *
             */
            POSE pos_EFB;
            /**
             * @brief JOINT Encoder feedback. Data from MXT_FB_JOINT
             *
             */
            JOINT jnt_EFB;
            /**
             * @brief PULSE Encoder feedback. Data from MXT_FB_PULSE
             *
             */
            PULSE pls_EFB;
            /**
             * @brief Current Command from robot controller to motors. Data from MXT_TYP_CMDCUR
             *
             */
            PULSE curr_cmd;
            /**
             * @brief Current Feedback from motors. Data from MXT_TYP_FBKCUR
             *
             */
            PULSE curr_FB;
            /**
             * @brief Monitored input/output signal data designation. MXT_IO_OUT for output signal. MXT_IO_IN for input signal
             *
             */
            uint16_t IOSendType;
            /**
             * @brief Head bit monitored
             *
             */
            uint16_t IOBitTop; // Variable for head bit number, BitTop.
            /**
             * @brief Transmission bit mask pattern designation (0x0000-0xffff), BitMask.
             *
             */
            uint16_t IOBitMask;
            /**
             * @brief Input/output signal data (0x0000-0xffff), IoData
             *
             */
            uint32_t IOBitData;
        } FB_packet;

        /**
         * @brief IP settings used in API.
         *
         */
        typedef struct
        {
            /**
             * @brief Generated. Socket descriptor
             *
             */
            #if __linux__
            int destSocket;
            #endif
            #if _WIN32
            SOCKET destSocket;
            #endif
            /**
             * @brief Generated. Structure for AF_INET
             *
             */
            struct sockaddr_in destSockAddr;
            /**
             * @brief User input. Destination IP address.
             *
             */
            std::string dst_ip_address;
            /**
             * @brief User input. Port number
             *
             */
            unsigned short port;
            /**
             * @brief Generated. net_addr retutn value. IPv4 network address in internet standard dot notation.
             *
             */
            int destAddr;
        } ip_settings;    // IP Settings Structure
        /**
         * @brief Send MXTsend to robot controller. Only 1 MXTsend per class object
         * @return int
         */
        int send_packet_();
        /**
         * @brief Receive MXTrecv from robot controller. Only 1 MXTrecv per class object
         * @return int
         */
        int recv_packet_();
        /* data */
    public:
        /**
         * @brief timeout value
        */
        int rtexc_timeout_= 100;
        /**
         * @brief Switch to debug mode. debug mode = 1. Normal = 0.
         *
         */
        int RT_API_DEBUG_MODE = 0;
        /**
         * @brief Track packet loss
         *
         */
        int packet_recv_lost;
        /**
         * @brief Transmission data counter value, CCount.
         *
         */
        int counter_;
        /**
         * @brief set robot controller clock cycle period. To calculate resolution for receving packets.
         *
         */
        float period = 3.5;
        /**
         * @brief true if robot connection is available. false if robot is disconnected.
         * 
         */
        bool robot_status = true;
        /**
         * @brief counts consecutive packet losses. Used to trigger robot_status.
         * 
         */
        int packet_lost_consec_cnt;
        /**
         * @brief Local command packet
         *
         */
        CMD_packet cmd_pack;
        /**
         * @brief robot ethernet settings
         *
         */
        ip_settings robot_ip;
        /**
         * @brief local feedback packet
         *
         */
        FB_packet fb_pack;
        /**
         * @brief Construct a rtexc::rtexc object. Initialize class variabes. If _WIN32, initialize WSAStartup. If debug, prints "Debug mode active."
         *
         */
        rtexc(float custom_cycle_time = 3.5);
        /**
         * @brief Create _WIN32 or UNIX UDP port using details in ip_settings this_port.
         * @return int
         */
        int create_port(); // generic create TCP/UDP port
        /**
         * @brief Initialize Real Time External Control function on robot controller. Uses NULL command so not robot will not move.
         * @brief Built in receive check and timeout
         * @return int
         */
        int WriteToRobot_init_(int buffer=100);
        /**
         * @brief Convert and arrange data from cmd_pack to MXTsend and sends packet.
         * @return int
         */
        int WriteToRobot_CMD_();
        /**
         * @brief Receive packet from robot controller. Convert and arrange data from MXTrecv to fb_pack.
         * @return int
         */
        int ReadFromRobot_FB_();
        /**
         * @brief Prints monitored data from feedback struct, FB_packet
         *
         * @return int
         */
        int print_monitored_feedback();
        /**
         * @brief Prints all data from feedback struct, FB_packet
         *
         * @return int
         */
        int print_all_feedback();
    };
}
#endif
/*EOF*/