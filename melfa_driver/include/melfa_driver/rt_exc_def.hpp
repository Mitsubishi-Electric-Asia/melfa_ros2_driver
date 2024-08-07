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
 * @file rt_exc_def.hpp
 * @author Liu Muyao
 * @brief Real time External Control Communication packet data structure definition
 * 
 * @copyright Apache License 2.0
 *
 */

#ifndef __RTEXC_DEF__
#define __RTEXC_DEF__

#include <stdint.h>
/**
 * @brief Joint coordinate system (Set unused additional axis to 0).
 *
 */
typedef struct
{
    /**
     * @brief J1 axis angle
     * @brief [rad]
     *
     */
    float j1;
    /**
     * @brief J2 axis angle
     * @brief [rad]
     *
     */
    float j2;
    /**
     * @brief J3 axis angle
     * @brief [rad]
     *
     */
    float j3;
    /**
     * @brief J4 axis angle
     * @brief [rad]
     *
     */
    float j4;
    /**
     * @brief J5 axis angle
     * @brief [rad]
     *
     */
    float j5;
    /**
     * @brief J6 axis angle
     * @brief [rad]
     *
     */
    float j6;
    /**
     * @brief Additional axis 1 (J7 axis angle)
     * @brief [rad]
     *
     */
    float j7;
    /**
     * @brief Additional axis 2 (J8 axis angle)
     * @brief [rad]
     *
     */
    float j8;
} JOINT;
/**
 * @brief XYZ coordinate system (Set unused additional axis to 0)
 *
 */
typedef struct
{
    /**
     * @brief X axis coordinate value
     * @brief [mm]
     *
     */
    float x;
    /**
     * @brief Y axis coordinate value
     * @brief [mm]
     *
     */
    float y;
    /**
     * @brief Z axis coordinate value
     * @brief [mm]
     *
     */
    float z;
    /**
     * @brief A axis coordinate value
     * @brief [rad]
     *
     */
    float a;
    /**
     * @brief B axis coordinate value
     * @brief [rad]
     *
     */
    float b;
    /**
     * @brief C axis coordinate value
     * @brief [rad]
     *
     */
    float c;
    /**
     * @brief Additional axis 1. Linear or Rotational
     * @brief [mm or rad]
     *
     */
    float l1;
    /**
     * @brief Additional axis 2 Linear or Rotational
     * @brief [mm or rad]
     *
     */
    float l2;
} WORLD;
/**
 * @brief POSE structure with WORLD structure and pose flags.
 *
 */
typedef struct
{
    WORLD w;
    /**
     * @brief Structural flag 1
     *
     */
    uint32_t sflg1;
    /**
     * @brief Structural flag 2
     *
     */
    uint32_t sflg2;

} POSE;

/**
 * @brief Pulse coordinate system (Set unused additional axis to 0)
 *
 */
typedef struct
{
    /**
     * @brief J1 motor pulse
     * @brief [pulse]
     *
     */
    int32_t p1;
    /**
     * @brief J2 motor pulse
     * @brief [pulse]
     *
     */
    int32_t p2;
    /**
     * @brief J3 motor pulse
     * @brief [pulse]
     *
     */
    int32_t p3;
    /**
     * @brief J4 motor pulse
     * @brief [pulse]
     *
     */
    int32_t p4;
    /**
     * @brief J5 motor pulse
     * @brief [pulse]
     *
     */
    int32_t p5;
    /**
     * @brief J6 motor pulse
     * @brief [pulse]
     *
     */
    int32_t p6;
    /**
     * @brief Additional axis 7 motor pulse
     * @brief [pulse]
     *
     */
    int32_t p7;
    /**
     * @brief Additional axis 8 motor pulse
     * @brief [pulse]
     *
     */
    int32_t p8;

} PULSE;
/************************************************************/
/* Data type ID
 */
/************************************************************/

/**
 * @brief Real Time External Control NULL command
 *
 */
#define MXT_CMD_NULL 0
/**
 * @brief Real Time External Control Move command
 *
 */
#define MXT_CMD_MOVE 1
/**
 * @brief Real Time External Control End command
 *
 */
#define MXT_CMD_END 255
// Command or monitor data type //
/**
 * @brief NULL data type
 *
 */
#define MXT_TYP_NULL 0

/*********
 * For the command and monitor
 *********/
/**
 * @brief Pose data type command
 *
 */
#define MXT_TYP_POSE 1
/**
 * @brief Joint data type command
 *
 */
#define MXT_TYP_JOINT 2
/**
 * @brief Pulse data type command
 *
 */
#define MXT_TYP_PULSE 3
/**
 * @brief Pose command type after filter
 *
 */
#define MXT_TYP_FPOSE 4
/**
 * @brief Joint command type after filter
 *
 */
#define MXT_TYP_FJOINT 5
/**
 * @brief Pulse command type after filter
 *
 */
#define MXT_TYP_FPULSE 6
/**
 * @brief Pose Encoder Feedback
 *
 */
#define MXT_TYP_FB_POSE 7
/**
 * @brief Joint Encoder Feedback
 *
 */
#define MXT_TYP_FB_JOINT 8
/**
 * @brief Pulse Encoder Feedback
 *
 */
#define MXT_TYP_FB_PULSE 9
// For current related monitors ////////////////////
/**
 * @brief Electric current command
 *
 */
#define MXT_TYP_CMDCUR 10
/**
 * @brief Electric current feedback
 *
 */
#define MXT_TYP_FBKCUR 11
/**************************
 * Signal data designation
 *************************/
/**
 * @brief No GPIO data
 *
 */
#define MXT_IO_NULL 0
/**
 * @brief GPIO output data
 *
 */
#define MXT_IO_OUT 1
/**
 * @brief GPIO input data
 *
 */
#define MXT_IO_IN 2

/**
 * @brief Real-time function communication data packet
 *
 */
typedef struct enet_rtcmd_str
{
    /**
     * @brief Command. MXT_CMD_NULL for NULL command. MXT_CMD_MOVE for Move command. MXT_CMD_END to end real time external control function in robot controller
     */
    uint16_t Command;
    /**
     * @brief Command data type designation. MXT_TYP_POSE for Pose. MXT_TYP_JOINT for joint. MXT_TYP_PULSE for pulse
     */
    uint16_t SendType;
    /**
     * @brief Monitor data type designation for first of four monitor data types
     *
     */
    uint16_t RecvType;
    /**
     * @brief Not used
     * 
     */
    uint16_t reserve;
    /**
     * @brief When used in MXTsend, Command data. When used in MXTrecv, Monitor data for first of four data types
     */
    union rtdata
    {
        POSE pos;  // XYZ type [mm/rad]
        JOINT jnt; // Joint type [rad]
        PULSE pls; // Pulse type [pulse]
    } dat;
    /**
     * @brief Send input/output signal data designation. MXT_IO_OUT for output signal. MXT_IO_NULL for input signal. When used in MXTsend, designate signal type of data sent. When used in MXTrecv, indicate signal type of data recevied.
     */
    uint16_t SendIOType;
    /**
     * @brief Return input/output signal data designation. MXT_IO_OUT for output signal. MXT_IO_NULL for input signal. When used in MXTsend, designate signal type of data received. Not used in MXTrecv
     */
    uint16_t RecvIOType;
    /**
     * @brief Head bit Number. When used in MXTsend, designate head bit of data sent. When used in MXTrecv. indicate head bit of data received
     */
    uint16_t BitTop;
    /**
     * @brief Transmission bit mask pattern designation (0x0000-0xffff). When used in MXTsend, bit mask for data sent. BitMask AND IoData = actual data sent. Not used in MXTrecv
     */
    uint16_t BitMask;
    /**
     * @brief Input/output signal data (0x0000-0xffff)
     *
     */
    uint16_t IoData;
    /**
     * @brief Timeout time counter value
     *
     */
    uint16_t TCount;
    /**
     * @brief Transmission data counter value. For MXTrecv only
     *
     */
    uint32_t CCount;
    /**
     * @brief Monitor data type designation for second of four monitor data types
     *
     */
    uint16_t RecvType1;
    /**
     * @brief Not used
     * 
     */
    uint16_t reserve1;
    /**
     * @brief When used in MXTsend, Command data. When used in MXTrecv, Monitor data for second of four data types.
     *
     */
    union rtdata1
    {              // Monitor data 1
        POSE pos;  // XYZ type [mm/rad]
        JOINT jnt; // JOINT type [rad]
        PULSE pls; // PULSE type [pulse] or Integer type [% / non-unit]
    } dat1;
    /**
     * @brief Monitor data type designation for third of four monitor data types
     *
     */
    uint16_t RecvType2;
    /**
     * @brief Not used
     * 
     */
    uint16_t reserve2;
    /**
     * @brief When used in MXTsend, Command data. When used in MXTrecv, Monitor data for third of four data types.
     *
     */
    union rtdata2
    {              // Monitor data 2
        POSE pos;  // XYZ type [mm/rad]
        JOINT jnt; // JOINT type [rad]
        PULSE pls; // PULSE type [pulse] or Integer type [% / non-unit]
    } dat2;
    /**
     * @brief Monitor data type designation for forth of four monitor data types
     *
     */
    uint16_t RecvType3;
    /**
     * @brief Not used
     * 
     */
    uint16_t reserve3;
    /**
     * @brief When used in MXTsend, Command data. When used in MXTrecv, Monitor data for forth of four data types.
     *
     */
    union rtdata3
    {              // Monitor data 3
        POSE pos;  // XYZ type [mm/rad]
        JOINT jnt; // JOINT type [rad]
        PULSE pls; // PULSE type [pulse] or Integer type [% / non-unit]
    } dat3;
} MXTCMD;

#endif
/*EOF*/
