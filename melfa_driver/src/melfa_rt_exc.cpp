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
 * @file melfa_rt_exc.cpp
 * @author Liu Muyao
 * @brief Source file for Real Time External Control API
 *
 * @copyright Apache License 2.0
 *
 */

#include "melfa_driver/melfa_rt_exc.hpp"

#if _WIN32
#pragma comment(lib, "Ws2_32.lib")
#endif

namespace MelfaEthernet
{

    rtexc::rtexc(float custom_cycle_time) : MXTsend{}, MXTrecv{}, console_msg{}, sendText{}, recvText{},
                                             packet_recv_lost(0), counter_(0)
    {
#ifdef _WIN32
        if (WSAStartup(MAKEWORD(1, 1), &Data) == SOCKET_ERROR)
        {
            std::cout << "Error initialising WSA.\n";
            system("pause");
        }

#endif
        memset(&cmd_pack, 0, sizeof(CMD_packet));
        memset(&fb_pack, 0, sizeof(FB_packet));

        if (RT_API_DEBUG_MODE)
            std::cout << "Debug mode Active.\n";
        period = custom_cycle_time;
    }

    int rtexc::create_port()
    {
        // IP Setting
        memset(&(robot_ip.destSockAddr), 0, sizeof(robot_ip.destSockAddr));
#if __linux__
        (robot_ip).destAddr = inet_addr((robot_ip).dst_ip_address.c_str()); // get IPv4 network address in standard dot format.
#endif
#if _WIN32
        (robot_ip).destAddr = inet_addr((robot_ip).dst_ip_address.c_str()); // get IPv4 network address in standard dot format.
#endif
        memcpy(&((robot_ip).destSockAddr.sin_addr), &((robot_ip).destAddr), sizeof((robot_ip).destAddr));
        (robot_ip).destSockAddr.sin_port = htons((robot_ip).port); // set port number
        (robot_ip).destSockAddr.sin_family = AF_INET;              // set standard struct
        // Creating Port
        if (RT_API_DEBUG_MODE)
        {
#if __linux__
            sprintf(console_msg, "Creating Socket......\nIP: %s\nPort: %d\n",
                    (robot_ip).dst_ip_address.c_str(), (robot_ip).port);
#endif
#if _WIN32
            sprintf_s(console_msg, "Creating Socket......\nIP: %s\nPort: %d\n",
                      (robot_ip).dst_ip_address.c_str(), (robot_ip).port);
#endif
            std::cout << console_msg;
        }

        (robot_ip).destSocket = socket(AF_INET, SOCK_DGRAM, 0); // UDP

#if __linux__
        int n = 512;
        if (setsockopt((robot_ip).destSocket, SOL_SOCKET, SO_RCVBUF, &n, sizeof(n)) == -1)
            if (RT_API_DEBUG_MODE)
                std::cerr << "WARNING: setsockopt FAILED\n";
#endif
        if ((robot_ip).destSocket < 0)
        {
            if (RT_API_DEBUG_MODE)
                std::cerr << "ERROR: Port Creation FAILED.\n";
            return -1;
        }
        else
        {
            if (RT_API_DEBUG_MODE)
                std::cerr << "Port Creation Successful.\n";
            return 0;
        }
    }

    int rtexc::WriteToRobot_init_(int buffer)
    {
        memset(&MXTsend, 0, sizeof(MXTsend));
        *type_mon = *cmd_pack.mon_dat;
        *(type_mon + 1) = *(cmd_pack.mon_dat + 1);
        *(type_mon + 2) = *(cmd_pack.mon_dat + 2);
        *(type_mon + 3) = *(cmd_pack.mon_dat + 3);
        MXTsend.Command = MXT_CMD_NULL;
        MXTsend.SendType = MXT_TYP_NULL;
        MXTsend.RecvType = (uint16_t)type_mon[0];
        MXTsend.RecvType1 = (uint16_t)type_mon[1];
        MXTsend.RecvType2 = (uint16_t)type_mon[2];
        MXTsend.RecvType3 = (uint16_t)type_mon[3];
        MXTsend.SendIOType = MXT_IO_NULL;
        MXTsend.RecvIOType = MXT_IO_NULL;
        MXTsend.BitTop = cmd_pack.IOBitTop;
        MXTsend.BitMask = cmd_pack.IOBitMask;
        MXTsend.IoData = cmd_pack.IOBitData;
        MXTsend.CCount = counter_;
        if (RT_API_DEBUG_MODE)
        {
            std::cout << "Monitoring Setting:\n";
            for (int i = 0; i < 4; i++)
            {
                switch (type_mon[i])
                {
                case MXT_TYP_JOINT:
                    std::cout << "Joint Command\n";
                    break;
                case MXT_TYP_FJOINT:
                    std::cout << "Joint Filtered Command\n";
                    break;
                case MXT_TYP_FB_JOINT:
                    std::cout << "Joint Feedback\n";
                    break;
                case MXT_TYP_POSE:
                    std::cout << "POSE Command\n";
                    break;
                case MXT_TYP_FPOSE:
                    std::cout << "POSE Filtered Command\n";
                    break;
                case MXT_TYP_FB_POSE:
                    std::cout << "POSE Feedback\n";
                    break;
                case MXT_TYP_PULSE:
                    std::cout << "PULSE Command\n";
                    break;
                case MXT_TYP_FPULSE:
                    std::cout << "PULSE Filtered Command\n";
                    break;
                case MXT_TYP_FB_PULSE:
                    std::cout << "PULSE Feedback\n";
                    break;
                case MXT_TYP_CMDCUR:
                    std::cout << "Current Command\n";
                    break;
                case MXT_TYP_FBKCUR:
                    std::cout << "Current Feedback\n";
                    break;
                case MXT_TYP_NULL:
                    std::cout << "NULL Monitoring Setting\n";
                    break;
                default:
                    std::cout << "Invalid Monitoring Setting";
                    break;
                }
            }
        }
        if (send_packet_() != 0)
            return -1;
        int j = 0;
        while (j < rtexc_timeout_)
        {
#if __linux__
            usleep(buffer);
#endif
            if (ReadFromRobot_FB_() != 0)
            {
                j++;
            }
            else
            {
                break;
            }
        }
        if (j >= rtexc_timeout_)
        {
            std::cout << "ERROR: First receive FAILED.\n";
            return -1;
        }
        return 0;
    }

    int rtexc::WriteToRobot_CMD_()
    {
        if (RT_API_DEBUG_MODE)
            std::cout << "Preparing packet.\n";
        memset(&MXTsend, 0, sizeof(MXTsend));
        if (cmd_pack.cmd_type == MXT_CMD_END)
        {
            MXTsend.Command = MXT_CMD_END;
            return send_packet_();
        }
        *type_mon = *cmd_pack.mon_dat;
        *(type_mon + 1) = *(cmd_pack.mon_dat + 1);
        *(type_mon + 2) = *(cmd_pack.mon_dat + 2);
        *(type_mon + 3) = *(cmd_pack.mon_dat + 3);
        MXTsend.Command = cmd_pack.cmd_type;
        MXTsend.SendType = cmd_pack.send_type;
        MXTsend.RecvType = (uint16_t)type_mon[0];
        MXTsend.RecvType1 = (uint16_t)type_mon[1];
        MXTsend.RecvType2 = (uint16_t)type_mon[2];
        MXTsend.RecvType3 = (uint16_t)type_mon[3];

        MXTsend.SendIOType = cmd_pack.IOSendType;
        MXTsend.RecvIOType = cmd_pack.IORecvType;
        MXTsend.BitTop = cmd_pack.IOBitTop;
        MXTsend.BitMask = cmd_pack.IOBitMask;
        MXTsend.IoData = cmd_pack.IOBitData;
        MXTsend.CCount = counter_;
        switch (cmd_pack.send_type)
        {
        case MXT_TYP_JOINT:
            MXTsend.dat.jnt = cmd_pack.jnt_CMD;
            break;
        case MXT_TYP_POSE:
            MXTsend.dat.pos = cmd_pack.pos_CMD;
            break;
        case MXT_TYP_PULSE:
            MXTsend.dat.pls = cmd_pack.pls_CMD;
            break;
        default:
            std::cerr << "ERROR: MXTsend send_type Data invalid.\n";
            break;
        }
        if (RT_API_DEBUG_MODE)
        {
            std::cout << cmd_pack.send_type << "\n";
            switch (MXTsend.SendType)
            {
            case MXT_TYP_JOINT:
                std::cout << "Sending JOINT CMD Data\n";
                memset(console_msg, 0, sizeof(console_msg));
#if __linux__
                sprintf(
#endif
#if _WIN32
                sprintf_s(
#endif
                    console_msg, "JOINT CMD Data: j1 = %f, j2 = %f, j3= %f, j4= %f, j5= %f, j6= %f, j7= %f, j8 = %f\n",
                          MXTsend.dat.jnt.j1,
                          MXTsend.dat.jnt.j2,
                          MXTsend.dat.jnt.j3,
                          MXTsend.dat.jnt.j4,
                          MXTsend.dat.jnt.j5,
                          MXTsend.dat.jnt.j6,
                          MXTsend.dat.jnt.j7,
                          MXTsend.dat.jnt.j8);
                std::cout << console_msg;
                break;
            case MXT_TYP_POSE:
                std::cout << "Sending POSE CMD Data.\n";
                memset(console_msg, 0, sizeof(console_msg));
#if __linux__
                sprintf(
#endif
#if _WIN32
                sprintf_s(
#endif
                    console_msg, "POSE CMD Data: x = %f, y = %f, z = %f, a = %f, b = %f, c= %f, l1= %f, l2= %f, sflg1 = %d, sflg2 = %d\n",
                          MXTsend.dat.pos.w.x,
                          MXTsend.dat.pos.w.y,
                          MXTsend.dat.pos.w.z,
                          MXTsend.dat.pos.w.a,
                          MXTsend.dat.pos.w.b,
                          MXTsend.dat.pos.w.c,
                          MXTsend.dat.pos.w.l1,
                          MXTsend.dat.pos.w.l2,
                          MXTsend.dat.pos.sflg1,
                          MXTsend.dat.pos.sflg2);
                std::cout << console_msg;
                break;
            case MXT_TYP_PULSE:
                std::cout << "Sending PULSE CMD Data.\n";
                memset(console_msg, 0, sizeof(console_msg));
#if __linux__
                sprintf(
#endif
#if _WIN32
                sprintf_s(
#endif
                    console_msg, "PULSE CMD Data: p1= %u, p1= %u, p3= %u, p4= %u, p5= %u, p6= %u, p7= %u, p8= %u\n",
                          MXTsend.dat.pls.p1,
                          MXTsend.dat.pls.p2,
                          MXTsend.dat.pls.p3,
                          MXTsend.dat.pls.p4,
                          MXTsend.dat.pls.p5,
                          MXTsend.dat.pls.p6,
                          MXTsend.dat.pls.p7,
                          MXTsend.dat.pls.p8);
                std::cout << console_msg;
                break;
            default:
                std::cerr << "WARNING: MXTsend send_type Data invalid.\n";
                break;
            }
            std::cout << "Monitoring Setting:\n";
            for (int i = 0; i < 4; i++)
            {
                switch (type_mon[i])
                {
                case MXT_TYP_JOINT:
                    std::cout << "Joint Command\n";
                    break;
                case MXT_TYP_FJOINT:
                    std::cout << "Joint Filtered Command\n";
                    break;
                case MXT_TYP_FB_JOINT:
                    std::cout << "Joint Feedback\n";
                    break;
                case MXT_TYP_POSE:
                    std::cout << "POSE Command\n";
                    break;
                case MXT_TYP_FPOSE:
                    std::cout << "POSE Filtered Command\n";
                    break;
                case MXT_TYP_FB_POSE:
                    std::cout << "POSE Feedback\n";
                    break;
                case MXT_TYP_PULSE:
                    std::cout << "PULSE Command\n";
                    break;
                case MXT_TYP_FPULSE:
                    std::cout << "PULSE Filtered Command\n";
                    break;
                case MXT_TYP_FB_PULSE:
                    std::cout << "PULSE Feedback\n";
                    break;
                case MXT_TYP_CMDCUR:
                    std::cout << "Current Command\n";
                    break;
                case MXT_TYP_FBKCUR:
                    std::cout << "Current Feedback\n";
                    break;
                case MXT_TYP_NULL:
                    std::cout << "NULL Monitoring Setting\n";
                    break;
                default:
                    std::cout << "Invalid Monitoring Setting";
                    break;
                }
            }
        }

        return send_packet_();
    }

    int rtexc::send_packet_()
    {
        int size;
        memset(sendText, 0, MAXBUFLEN);
        memcpy(sendText, &MXTsend, sizeof(MXTsend));

        if (RT_API_DEBUG_MODE)
        {
            memset(console_msg, 0, sizeof(console_msg));
#if __linux__
            sprintf(console_msg, "Sending to: destAddr=%d, destSocket=%d, dst_ip_address = %s, port= %d.\n",
                    robot_ip.destAddr, robot_ip.destSocket, robot_ip.dst_ip_address.c_str(), robot_ip.port);
#elif _WIN32
            sprintf_s(console_msg, "Sending to: destAddr=%d, destSocket=%d, dst_ip_address = %s, port= %d.\n",
                      robot_ip.destAddr, (int)robot_ip.destSocket, robot_ip.dst_ip_address.c_str(), robot_ip.port);
#endif
            std::cout << console_msg;
        }

        size = sendto(robot_ip.destSocket, sendText, sizeof(MXTCMD), NO_FLAGS_SET, (struct sockaddr *)&(robot_ip.destSockAddr), sizeof((robot_ip.destSockAddr)));
        if (size != sizeof(MXTCMD))
        {
            if (RT_API_DEBUG_MODE)
            {
                memset(console_msg, 0, sizeof(console_msg));
#if __linux__
                sprintf(console_msg, "errno= %s,size = %d, size of MXTCMD = %ld.\n", strerror(errno), size, sizeof(MXTCMD));
#endif
#if _WIN32
                sprintf_s(console_msg, "errno= %s,size = %d, size of MXTCMD = %lld.\n", strerror(errno), size, sizeof(MXTCMD));
#endif
                std::cerr << "Error: Send packet FAILED.\n"
                          << console_msg;
            }
            return -1;
        }
        else
        {
            if (RT_API_DEBUG_MODE)
                std::cout << "Send Packet Size: " << size << "\n";
        }
        if (size < 0)
        {
            if (RT_API_DEBUG_MODE)
            {
                memset(console_msg, 0, sizeof(console_msg));
#ifdef __linux__
                sprintf(console_msg, "errno= %s,size = %d, size of MXTCMD = %ld.\n", strerror(errno), size, sizeof(MXTCMD));
#endif
#ifdef _WIN32
                sprintf_s(console_msg, "WSAerro= %s,size = %d, size of MXTCMD = %lld.\n", strerror(errno), size, sizeof(MXTCMD));
#endif
                std::cerr << "Error: Send packet FAILED.\n"
                          << console_msg;
            }
            return -1;
        }
        if (RT_API_DEBUG_MODE)
            std::cout << "Packet sent.\n";
        counter_++;
        return 0;
    }

    int rtexc::ReadFromRobot_FB_()
    {
        if (recv_packet_() == 0)
        {
            if (RT_API_DEBUG_MODE)
                std::cout << "Packet Received. Processing....\n";
            packet_lost_consec_cnt = 0;
        }
        else
        {
            if (RT_API_DEBUG_MODE)
                std::cout << "Packet receive failed\n";
            packet_lost_consec_cnt +=1;
            if(packet_lost_consec_cnt>=rtexc_timeout_)
                robot_status = false;
            
            return -1;
        }
        memset(&MXTrecv, 0, sizeof(MXTrecv));
        memcpy(&MXTrecv, &recvText, sizeof(MXTrecv));
        char str[20];
        fb_pack.IOSendType = MXTrecv.SendIOType;
        fb_pack.IOBitTop = MXTrecv.BitTop;
        fb_pack.IOBitMask = MXTrecv.BitMask;
        fb_pack.IOBitData = MXTrecv.IoData;
        if (RT_API_DEBUG_MODE)
        {
#if __linux__
            if (MXTrecv.SendIOType == MXT_IO_IN)
                sprintf(str, "INPUT %d:%04x", fb_pack.IOBitTop, fb_pack.IOBitData);
            else if (MXTrecv.SendIOType == MXT_IO_OUT)
                sprintf(str, "OUTPUT %d:%04x", fb_pack.IOBitTop, fb_pack.IOBitData);
            else
                sprintf(str, "Warning: NO IO DATA");
#endif
#if _WIN32
            if (MXTrecv.SendIOType == MXT_IO_IN)
                sprintf_s(str, "INPUT %d:%04x", fb_pack.IOBitTop, fb_pack.IOBitData);
            else if (MXTrecv.SendIOType == MXT_IO_OUT)
                sprintf_s(str, "OUTPUT %d:%04x", fb_pack.IOBitTop, fb_pack.IOBitData);
            else
                sprintf_s(str, "Warning: NO IO DATA");
#endif
            std::cout << "IO Monitoring: " << str << std::endl;
        }
        switch (MXTrecv.RecvType)
        {
        case MXT_TYP_JOINT:
            if (RT_API_DEBUG_MODE)
                std::cout << "Reading JOINT command values\n";
            fb_pack.jnt_FB = MXTrecv.dat.jnt;
            break;
        case MXT_TYP_FJOINT:
            if (RT_API_DEBUG_MODE)
                std::cout << "Reading filtered JOINT command values\n";
            fb_pack.jnt_FFB = MXTrecv.dat.jnt;
            break;
        case MXT_TYP_FB_JOINT:
            if (RT_API_DEBUG_MODE)
                std::cout << "Reading JOINT encoder values\n";
            fb_pack.jnt_EFB = MXTrecv.dat.jnt;
            break;
        case MXT_TYP_POSE:
            if (RT_API_DEBUG_MODE)
                std::cout << "Reading POSE command values\n";
            fb_pack.pos_FB = MXTrecv.dat.pos;
            break;
        case MXT_TYP_FPOSE:
            if (RT_API_DEBUG_MODE)
                std::cout << "Reading filtered POSE command values\n";
            fb_pack.pos_EFB = MXTrecv.dat.pos;
            break;
        case MXT_TYP_FB_POSE:
            if (RT_API_DEBUG_MODE)
                std::cout << "Reading POSE encoder values\n";
            fb_pack.pos_EFB = MXTrecv.dat.pos;
            break;
        case MXT_TYP_PULSE:
            if (RT_API_DEBUG_MODE)
                std::cout << "Reading PULSE command values\n";
            fb_pack.pls_FB = MXTrecv.dat.pls;
            break;
        case MXT_TYP_FPULSE:
            if (RT_API_DEBUG_MODE)
                std::cout << "Reading filtered PULSE command values\n";
            fb_pack.pls_FFB = MXTrecv.dat.pls;
            break;
        case MXT_TYP_FB_PULSE:
            if (RT_API_DEBUG_MODE)
                std::cout << "Reading PULSE encoder values\n";
            fb_pack.pls_EFB = MXTrecv.dat.pls;
            break;
        case MXT_TYP_CMDCUR:
            if (RT_API_DEBUG_MODE)
                std::cout << "Reading CURRENT command values\n";
            fb_pack.curr_cmd = MXTrecv.dat.pls;
            break;
        case MXT_TYP_FBKCUR:
            if (RT_API_DEBUG_MODE)
                std::cout << "Reading CURRENT feedback values\n";
            fb_pack.curr_FB = MXTrecv.dat.pls;
            break;
        case MXT_TYP_NULL:
            if (RT_API_DEBUG_MODE)
            {
#if __linux__
                sprintf(console_msg, "Receive (%u): TCount=%d Type(NULL)=%d (%s)", MXTrecv.CCount, MXTrecv.TCount, 0, str);
#endif
#if _WIN32
                sprintf_s(console_msg, "Receive (%u): TCount=%d Type(NULL)=%d (%s)", MXTrecv.CCount, MXTrecv.TCount, 0, str);
#endif
                std::cout << console_msg << std::endl;
            }
            break;
        default:
            if (RT_API_DEBUG_MODE)
                std::cout << "Bad data type.¥n" << std::endl;
            break;
        }
        switch (MXTrecv.RecvType1)
        {
        case MXT_TYP_JOINT:
            if (RT_API_DEBUG_MODE)
                std::cout << "Reading JOINT command values\n";
            fb_pack.jnt_FB = MXTrecv.dat1.jnt;
            break;
        case MXT_TYP_FJOINT:
            if (RT_API_DEBUG_MODE)
                std::cout << "Reading filtered JOINT command values\n";
            fb_pack.jnt_FFB = MXTrecv.dat1.jnt;
            break;
        case MXT_TYP_FB_JOINT:
            if (RT_API_DEBUG_MODE)
                std::cout << "Reading JOINT encoder values\n";
            fb_pack.jnt_EFB = MXTrecv.dat1.jnt;
            break;
        case MXT_TYP_POSE:
            if (RT_API_DEBUG_MODE)
                std::cout << "Reading POSE command values\n";
            fb_pack.pos_FB = MXTrecv.dat1.pos;
            break;
        case MXT_TYP_FPOSE:
            if (RT_API_DEBUG_MODE)
                std::cout << "Reading filtered POSE command values\n";
            fb_pack.pos_EFB = MXTrecv.dat1.pos;
            break;
        case MXT_TYP_FB_POSE:
            if (RT_API_DEBUG_MODE)
                std::cout << "Reading POSE encoder values\n";
            fb_pack.pos_EFB = MXTrecv.dat1.pos;
            break;
        case MXT_TYP_PULSE:
            if (RT_API_DEBUG_MODE)
                std::cout << "Reading PULSE command values\n";
            fb_pack.pls_FB = MXTrecv.dat1.pls;
            break;
        case MXT_TYP_FPULSE:
            if (RT_API_DEBUG_MODE)
                std::cout << "Reading filtered PULSE command values\n";
            fb_pack.pls_FFB = MXTrecv.dat1.pls;
            break;
        case MXT_TYP_FB_PULSE:
            if (RT_API_DEBUG_MODE)
                std::cout << "Reading PULSE encoder values\n";
            fb_pack.pls_EFB = MXTrecv.dat1.pls;
            break;
        case MXT_TYP_CMDCUR:
            if (RT_API_DEBUG_MODE)
                std::cout << "Reading CURRENT command values\n";
            fb_pack.curr_cmd = MXTrecv.dat1.pls;
            break;
        case MXT_TYP_FBKCUR:
            if (RT_API_DEBUG_MODE)
                std::cout << "Reading CURRENT feedback values\n";
            fb_pack.curr_FB = MXTrecv.dat1.pls;
            break;
        case MXT_TYP_NULL:
            if (RT_API_DEBUG_MODE)
            {
#if __linux__
                sprintf(console_msg, "Receive (%u): TCount=%d Type(NULL)=%d (%s)", MXTrecv.CCount, MXTrecv.TCount, 0, str);
#endif
#if _WIN32
                sprintf_s(console_msg, "Receive (%u): TCount=%d Type(NULL)=%d (%s)", MXTrecv.CCount, MXTrecv.TCount, 0, str);
#endif
                std::cout << console_msg << std::endl;
            }
            break;
        default:
            std::cout << "Bad data type.¥n" << std::endl;
            break;
        }
        switch (MXTrecv.RecvType2)
        {
        case MXT_TYP_JOINT:
            if (RT_API_DEBUG_MODE)
                std::cout << "Reading JOINT command values\n";
            fb_pack.jnt_FB = MXTrecv.dat2.jnt;
            break;
        case MXT_TYP_FJOINT:
            if (RT_API_DEBUG_MODE)
                std::cout << "Reading filtered JOINT command values\n";
            fb_pack.jnt_FFB = MXTrecv.dat2.jnt;
            break;
        case MXT_TYP_FB_JOINT:
            if (RT_API_DEBUG_MODE)
                std::cout << "Reading JOINT encoder values\n";
            fb_pack.jnt_EFB = MXTrecv.dat2.jnt;
            break;
        case MXT_TYP_POSE:
            if (RT_API_DEBUG_MODE)
                std::cout << "Reading POSE command values\n";
            fb_pack.pos_FB = MXTrecv.dat2.pos;
            break;
        case MXT_TYP_FPOSE:
            if (RT_API_DEBUG_MODE)
                std::cout << "Reading filtered POSE command values\n";
            fb_pack.pos_EFB = MXTrecv.dat2.pos;
            break;
        case MXT_TYP_FB_POSE:
            if (RT_API_DEBUG_MODE)
                std::cout << "Reading POSE encoder values\n";
            fb_pack.pos_EFB = MXTrecv.dat2.pos;
            break;
        case MXT_TYP_PULSE:
            if (RT_API_DEBUG_MODE)
                std::cout << "Reading PULSE command values\n";
            fb_pack.pls_FB = MXTrecv.dat2.pls;
            break;
        case MXT_TYP_FPULSE:
            if (RT_API_DEBUG_MODE)
                std::cout << "Reading filtered PULSE command values\n";
            fb_pack.pls_FFB = MXTrecv.dat2.pls;
            break;
        case MXT_TYP_FB_PULSE:
            if (RT_API_DEBUG_MODE)
                std::cout << "Reading PULSE encoder values\n";
            fb_pack.pls_EFB = MXTrecv.dat2.pls;
            break;
        case MXT_TYP_CMDCUR:
            if (RT_API_DEBUG_MODE)
                std::cout << "Reading CURRENT command values\n";
            fb_pack.curr_cmd = MXTrecv.dat2.pls;
            break;
        case MXT_TYP_FBKCUR:
            if (RT_API_DEBUG_MODE)
                std::cout << "Reading CURRENT feedback values\n";
            fb_pack.curr_FB = MXTrecv.dat2.pls;
            break;
        case MXT_TYP_NULL:
            if (RT_API_DEBUG_MODE)
            {
#if __linux__
                sprintf(console_msg, "Receive (%u): TCount=%d Type(NULL)=%d (%s)", MXTrecv.CCount, MXTrecv.TCount, 0, str);
#endif
#if _WIN32
                sprintf_s(console_msg, "Receive (%u): TCount=%d Type(NULL)=%d (%s)", MXTrecv.CCount, MXTrecv.TCount, 0, str);
#endif
                std::cout << console_msg << std::endl;
            }
            break;
        default:
            if (RT_API_DEBUG_MODE)
                std::cout << "Bad data type.¥n" << std::endl;
            break;
        }
        switch (MXTrecv.RecvType3)
        {
        case MXT_TYP_JOINT:
            if (RT_API_DEBUG_MODE)
                std::cout << "Reading JOINT command values\n";
            fb_pack.jnt_FB = MXTrecv.dat3.jnt;
            break;
        case MXT_TYP_FJOINT:
            if (RT_API_DEBUG_MODE)
                std::cout << "Reading filtered JOINT command values\n";
            fb_pack.jnt_FFB = MXTrecv.dat3.jnt;
            break;
        case MXT_TYP_FB_JOINT:
            if (RT_API_DEBUG_MODE)
                std::cout << "Reading JOINT encoder values\n";
            fb_pack.jnt_EFB = MXTrecv.dat3.jnt;
            break;
        case MXT_TYP_POSE:
            if (RT_API_DEBUG_MODE)
                std::cout << "Reading POSE command values\n";
            fb_pack.pos_FB = MXTrecv.dat3.pos;
            break;
        case MXT_TYP_FPOSE:
            if (RT_API_DEBUG_MODE)
                std::cout << "Reading filtered POSE command values\n";
            fb_pack.pos_EFB = MXTrecv.dat3.pos;
            break;
        case MXT_TYP_FB_POSE:
            if (RT_API_DEBUG_MODE)
                std::cout << "Reading POSE encoder values\n";
            fb_pack.pos_EFB = MXTrecv.dat3.pos;
            break;
        case MXT_TYP_PULSE:
            if (RT_API_DEBUG_MODE)
                std::cout << "Reading PULSE command values\n";
            fb_pack.pls_FB = MXTrecv.dat3.pls;
            break;
        case MXT_TYP_FPULSE:
            if (RT_API_DEBUG_MODE)
                std::cout << "Reading filtered PULSE command values\n";
            fb_pack.pls_FFB = MXTrecv.dat3.pls;
            break;
        case MXT_TYP_FB_PULSE:
            if (RT_API_DEBUG_MODE)
                std::cout << "Reading PULSE encoder values\n";
            fb_pack.pls_EFB = MXTrecv.dat3.pls;
            break;
        case MXT_TYP_CMDCUR:
            if (RT_API_DEBUG_MODE)
                std::cout << "Reading CURRENT command values\n";
            fb_pack.curr_cmd = MXTrecv.dat3.pls;

            break;
        case MXT_TYP_FBKCUR:
            if (RT_API_DEBUG_MODE)
                std::cout << "Reading CURRENT feedback values\n";
            fb_pack.curr_FB = MXTrecv.dat3.pls;
            break;
        case MXT_TYP_NULL:
            if (RT_API_DEBUG_MODE)
            {
#if __linux__
                sprintf(console_msg, "Receive (%u): TCount=%d Type(NULL)=%d (%s)", MXTrecv.CCount, MXTrecv.TCount, 0, str);
#endif
#if _WIN32
                sprintf_s(console_msg, "Receive (%u): TCount=%d Type(NULL)=%d (%s)", MXTrecv.CCount, MXTrecv.TCount, 0, str);
#endif
                std::cout << console_msg << std::endl;
            }
            break;
        default:
            if (RT_API_DEBUG_MODE)
                std::cout << "Bad data type." << std::endl;
            break;
        }
        return 0;
    }
    int rtexc::recv_packet_()
    {
        memset(recvText, 0, MAXBUFLEN);
        int numrcv;
        fd_set SockSet;
        FD_ZERO(&SockSet);                     // SockSet initialization
        FD_SET(robot_ip.destSocket, &SockSet); // SockSet registration
        int status;
        memset(console_msg, 0, sizeof(console_msg));
        if (RT_API_DEBUG_MODE)
        {
            memset(console_msg, 0, sizeof(console_msg));
#if __linux__
            sprintf(console_msg, "Receiving from: destAddr=%d, destSocket=%d, dst_ip_address = %s, port= %d.\n",
                    robot_ip.destAddr, robot_ip.destSocket, robot_ip.dst_ip_address.c_str(), robot_ip.port);
#endif
#if _WIN32
            sprintf_s(console_msg, "Receiving from: destAddr=%d, destSocket=%d, dst_ip_address = %s, port= %d.\n",
                      robot_ip.destAddr, (int)robot_ip.destSocket, robot_ip.dst_ip_address.c_str(), robot_ip.port);
#endif
            std::cout << console_msg;
        }

        sTimeOut.tv_sec = 0;
        sTimeOut.tv_usec = (long)(2 * period * 1000);

#ifdef _WIN32
        status = select(0, &SockSet, (fd_set *)NULL, (fd_set *)NULL, &sTimeOut);
#endif
#ifdef __linux__
        status = select(robot_ip.destSocket + 4, &SockSet, (fd_set *)NULL, (fd_set *)NULL, &sTimeOut);
#endif
        if (status < 0)
        {
            if (RT_API_DEBUG_MODE)
            {
                std::cerr << "ERROR: Select Receive Socket FAILED.\t" << std::endl;
#ifdef _WIN32
                std::cerr << "WSAGetLastError: " << WSAGetLastError() << std::endl;
#endif
#ifdef __linux__
                std::cerr << "ERRNO: " << strerror(errno) << std::endl;
#endif
            }
            packet_recv_lost++;
            return -1;
        }
        if ((status > 0) && (FD_ISSET(robot_ip.destSocket, &SockSet) != 0))
        {
            numrcv = recvfrom(robot_ip.destSocket, recvText, MAXBUFLEN, 0, NULL, NULL);
            if (numrcv < 0)
            {
                if (RT_API_DEBUG_MODE)
                    std::cerr << "ERROR: Receive FAILED.\n";
                packet_recv_lost++;
                return -1;
            }
            if (RT_API_DEBUG_MODE)
                std::cout << "Receive Packet Size: " << numrcv << "\n";
            return 0;
        }
        else
        {
            if (RT_API_DEBUG_MODE)
                std::cerr << "ERROR: Condition [status>0)] and [FD_ISSET(robot_ip.destSocket,&SockSet)!=0)] FAILED.\n";
            if (status == 0)
            {
                if (RT_API_DEBUG_MODE)
                    std::cerr << "Time limit expired.\tstatus:" << status << std::endl;
            }
            packet_recv_lost++;
            return -1;
        }
    }
    int rtexc::print_monitored_feedback()
    {
        memset(console_msg, 0, sizeof(console_msg));
#if __linux__
        sprintf(
#endif
#if _WIN32
        sprintf_s(
#endif
        console_msg,"I/O Type: %s\nHead Bit: %d\nBit Mask: %4x\nBit Data: %4x\n",fb_pack.IOSendType==MXT_IO_IN ? "Input" : (fb_pack.IOSendType==MXT_IO_OUT ? "Output" :" NULL"),fb_pack.IOBitTop,fb_pack.IOBitMask,fb_pack.IOBitData);
        std::cout<<console_msg;
        for (int i = 0; i < 4; i++)
        {
            memset(console_msg, 0, sizeof(console_msg));
            switch (type_mon[i])
            {
            case MXT_TYP_JOINT:
#if __linux__
                sprintf(
#endif
#if _WIN32
                sprintf_s(
#endif
                    console_msg, "JOINT Command Feedback: j1= %f, j2= %f, j3= %f, j4= %f, j5= %f, j6= %f, j7= %f, j8= %f\n",
                          fb_pack.jnt_FB.j1,
                          fb_pack.jnt_FB.j2,
                          fb_pack.jnt_FB.j3,
                          fb_pack.jnt_FB.j4,
                          fb_pack.jnt_FB.j5,
                          fb_pack.jnt_FB.j6,
                          fb_pack.jnt_FB.j7,
                          fb_pack.jnt_FB.j8);

                std::cout << console_msg;
                break;
            case MXT_TYP_FJOINT:
#if __linux__
                sprintf(
#endif
#if _WIN32
                sprintf_s(
#endif
                    console_msg, "JOINT filtered Command Feedback: j1= %f, j2= %f, j3= %f, j4= %f, j5= %f, j6= %f, j7= %f, j8= %f\n",
                          fb_pack.jnt_FFB.j1,
                          fb_pack.jnt_FFB.j2,
                          fb_pack.jnt_FFB.j3,
                          fb_pack.jnt_FFB.j4,
                          fb_pack.jnt_FFB.j5,
                          fb_pack.jnt_FFB.j6,
                          fb_pack.jnt_FFB.j7,
                          fb_pack.jnt_FFB.j8);

                std::cout << console_msg;
                break;
            case MXT_TYP_FB_JOINT:
#if __linux__
                sprintf(
#endif
#if _WIN32
                sprintf_s(
#endif
                    console_msg, "JOINT Encoder Feedback: j1= %f, j2= %f, j3= %f, j4= %f, j5= %f, j6= %f, j7= %f, j8= %f\n",
                          fb_pack.jnt_EFB.j1,
                          fb_pack.jnt_EFB.j2,
                          fb_pack.jnt_EFB.j3,
                          fb_pack.jnt_EFB.j4,
                          fb_pack.jnt_EFB.j5,
                          fb_pack.jnt_EFB.j6,
                          fb_pack.jnt_EFB.j7,
                          fb_pack.jnt_EFB.j8);
                std::cout << console_msg;
                break;
            case MXT_TYP_POSE:
#if __linux__
                sprintf(
#endif
#if _WIN32
                sprintf_s(
#endif
                    console_msg, "POSE Commmand Feedback: x= %f, y= %f, z= %f, a= %f, b= %f, c= %f, l1= %f, l2= %f, sflg1= %d, sflg2= %d\n",
                          fb_pack.pos_FB.w.x,
                          fb_pack.pos_FB.w.y,
                          fb_pack.pos_FB.w.z,
                          fb_pack.pos_FB.w.a,
                          fb_pack.pos_FB.w.b,
                          fb_pack.pos_FB.w.c,
                          fb_pack.pos_FB.w.l1,
                          fb_pack.pos_FB.w.l2,
                          fb_pack.pos_FB.sflg1,
                          fb_pack.pos_FB.sflg2);

                std::cout << console_msg;
                break;
            case MXT_TYP_FPOSE:
#if __linux__
                sprintf(
#endif
#if _WIN32
                sprintf_s(
#endif

                    console_msg, "POSE Filtered Command Feedback: x= %f, y= %f, z= %f, a= %f, b= %f, c= %f, l1= %f, l2= %f, sflg1= %d, sflg2= %d\n",
                          fb_pack.pos_FFB.w.x,
                          fb_pack.pos_FFB.w.y,
                          fb_pack.pos_FFB.w.z,
                          fb_pack.pos_FFB.w.a,
                          fb_pack.pos_FFB.w.b,
                          fb_pack.pos_FFB.w.c,
                          fb_pack.pos_FFB.w.l1,
                          fb_pack.pos_FFB.w.l2,
                          fb_pack.pos_FFB.sflg1,
                          fb_pack.pos_FFB.sflg2);
                std::cout << console_msg;
                break;
            case MXT_TYP_FB_POSE:
#if __linux__
                sprintf(
#endif
#if _WIN32
                sprintf_s(
#endif

                    console_msg, "POSE Encoder Feedback: x= %f, y= %f, z= %f, a= %f, b= %f, c= %f, l1= %f, l2= %f, sflg1= %d, sflg2= %d\n",
                          fb_pack.pos_EFB.w.x,
                          fb_pack.pos_EFB.w.y,
                          fb_pack.pos_EFB.w.z,
                          fb_pack.pos_EFB.w.a,
                          fb_pack.pos_EFB.w.b,
                          fb_pack.pos_EFB.w.c,
                          fb_pack.pos_EFB.w.l1,
                          fb_pack.pos_EFB.w.l2,
                          fb_pack.pos_EFB.sflg1,
                          fb_pack.pos_EFB.sflg2);
                std::cout << console_msg;
                break;
            case MXT_TYP_PULSE:
#if __linux__
                sprintf(
#endif
#if _WIN32
                sprintf_s(
                    #endif
                    console_msg, "PULSE Command Feedback: p1= %d, p1= %d, p3= %d, p4= %d, p5= %d, p6= %d, p7= %d, p8= %d\n",
                          fb_pack.pls_FB.p1,
                          fb_pack.pls_FB.p2,
                          fb_pack.pls_FB.p3,
                          fb_pack.pls_FB.p4,
                          fb_pack.pls_FB.p5,
                          fb_pack.pls_FB.p6,
                          fb_pack.pls_FB.p7,
                          fb_pack.pls_FB.p8);
                std::cout << console_msg;
                break;
            case MXT_TYP_FPULSE:
#if __linux__
                sprintf(
#endif
#if _WIN32
                sprintf_s(
                    #endif
                    console_msg, "PULSE Filtered Command Feedback: p1= %d, p1= %d, p3= %d, p4= %d, p5= %d, p6= %d, p7= %d, p8= %d\n",
                          fb_pack.pls_FFB.p1,
                          fb_pack.pls_FFB.p2,
                          fb_pack.pls_FFB.p3,
                          fb_pack.pls_FFB.p4,
                          fb_pack.pls_FFB.p5,
                          fb_pack.pls_FFB.p6,
                          fb_pack.pls_FFB.p7,
                          fb_pack.pls_FFB.p8);
                std::cout << console_msg;
                break;
            case MXT_TYP_FB_PULSE:
#if __linux__
                sprintf(
#endif
#if _WIN32
                sprintf_s(
#endif
                    console_msg, "PULSE Encoder Feedback: p1= %d, p1= %d, p3= %d, p4= %d, p5= %d, p6= %d, p7= %d, p8= %d\n",
                          fb_pack.pls_EFB.p1,
                          fb_pack.pls_EFB.p2,
                          fb_pack.pls_EFB.p3,
                          fb_pack.pls_EFB.p4,
                          fb_pack.pls_EFB.p5,
                          fb_pack.pls_EFB.p6,
                          fb_pack.pls_EFB.p7,
                          fb_pack.pls_EFB.p8);
                std::cout << console_msg;
                break;
            case MXT_TYP_CMDCUR:
#if __linux__
                sprintf(
#endif
#if _WIN32
                sprintf_s(
#endif
                    console_msg, "CURRENT Command Feedback: p1= %d, p1= %d, p3= %d, p4= %d, p5= %d, p6= %d, p7= %d, p8= %d\n",
                          fb_pack.curr_cmd.p1,
                          fb_pack.curr_cmd.p2,
                          fb_pack.curr_cmd.p3,
                          fb_pack.curr_cmd.p4,
                          fb_pack.curr_cmd.p5,
                          fb_pack.curr_cmd.p6,
                          fb_pack.curr_cmd.p7,
                          fb_pack.curr_cmd.p8);
                std::cout << console_msg;
                break;
            case MXT_TYP_FBKCUR:
#if __linux__
                sprintf(
#endif
#if _WIN32
                sprintf_s(
#endif
                    console_msg, "CURRENT Actual Feedback: p1= %d, p1= %d, p3= %d, p4= %d, p5= %d, p6= %d, p7= %d, p8= %d\n",
                          fb_pack.curr_FB.p1,
                          fb_pack.curr_FB.p2,
                          fb_pack.curr_FB.p3,
                          fb_pack.curr_FB.p4,
                          fb_pack.curr_FB.p5,
                          fb_pack.curr_FB.p6,
                          fb_pack.curr_FB.p7,
                          fb_pack.curr_FB.p8);
                std::cout << console_msg;
                break;
            case MXT_TYP_NULL:
                memset(console_msg, 0, sizeof(console_msg));
#if __linux__
                sprintf(
#endif
#if _WIN32
                sprintf_s(
#endif
                    console_msg, "Receive (%u): TCount=%d Type(NULL)=%d ", MXTrecv.CCount, MXTrecv.TCount, 0);
                std::cout << console_msg << std::endl;
                break;
            default:
                if (RT_API_DEBUG_MODE)
                    std::cout << "Bad data type.$n" << std::endl;
                break;
            }
        }
        std::cout << "Packet Sent: " << counter_ << "\n";
        std::cout << "Packet lost: " << packet_recv_lost << "\n";
        return 0;
    }
    int rtexc::print_all_feedback()
    {
        memset(console_msg, 0, sizeof(console_msg));
#if __linux__
        sprintf(
#endif
#if _WIN32
        sprintf_s(
#endif
        console_msg,"I/O Type: %s\nHead Bit: %d\nBit Mask: %4x\nBit Data: %4x\n",fb_pack.IOSendType==MXT_IO_IN ? "Input" : (fb_pack.IOSendType==MXT_IO_OUT ? "Output" :" NULL"),fb_pack.IOBitTop,fb_pack.IOBitMask,fb_pack.IOBitData);
        std::cout<<console_msg;

#if __linux__
        sprintf(
#endif
#if _WIN32
        sprintf_s(
#endif
            console_msg, "JOINT Command Feedback: j1= %f, j2= %f, j3= %f, j4= %f, j5= %f, j6= %f, j7= %f, j8= %f\n",
                  fb_pack.jnt_FB.j1,
                  fb_pack.jnt_FB.j2,
                  fb_pack.jnt_FB.j3,
                  fb_pack.jnt_FB.j4,
                  fb_pack.jnt_FB.j5,
                  fb_pack.jnt_FB.j6,
                  fb_pack.jnt_FB.j7,
                  fb_pack.jnt_FB.j8);
        std::cout << console_msg;
#if __linux__
        sprintf(
#endif
#if _WIN32
        sprintf_s(
#endif
            console_msg, "JOINT filtered Command Feedback: j1= %f, j2= %f, j3= %f, j4= %f, j5= %f, j6= %f, j7= %f, j8= %f\n",
                  fb_pack.jnt_FFB.j1,
                  fb_pack.jnt_FFB.j2,
                  fb_pack.jnt_FFB.j3,
                  fb_pack.jnt_FFB.j4,
                  fb_pack.jnt_FFB.j5,
                  fb_pack.jnt_FFB.j6,
                  fb_pack.jnt_FFB.j7,
                  fb_pack.jnt_FFB.j8);
        std::cout << console_msg;
#if __linux__
        sprintf(
#endif
#if _WIN32
        sprintf_s(
#endif
            console_msg, "JOINT Encoder Feedback: j1= %f, j2= %f, j3= %f, j4= %f, j5= %f, j6= %f, j7= %f, j8= %f\n",
                  fb_pack.jnt_EFB.j1,
                  fb_pack.jnt_EFB.j2,
                  fb_pack.jnt_EFB.j3,
                  fb_pack.jnt_EFB.j4,
                  fb_pack.jnt_EFB.j5,
                  fb_pack.jnt_EFB.j6,
                  fb_pack.jnt_EFB.j7,
                  fb_pack.jnt_EFB.j8);
        std::cout << console_msg;
#if __linux__
        sprintf(
#endif
#if _WIN32
        sprintf_s(
#endif
            console_msg, "POSE Commmand Feedback: x= %f, y= %f, z= %f, a= %f, b= %f, c= %f, l1= %f, l2= %f, sflg1= %d, sflg2= %d\n",
                  fb_pack.pos_FB.w.x,
                  fb_pack.pos_FB.w.y,
                  fb_pack.pos_FB.w.z,
                  fb_pack.pos_FB.w.a,
                  fb_pack.pos_FB.w.b,
                  fb_pack.pos_FB.w.c,
                  fb_pack.pos_FB.w.l1,
                  fb_pack.pos_FB.w.l2,
                  fb_pack.pos_FB.sflg1,
                  fb_pack.pos_FB.sflg2);
        std::cout << console_msg;
#if __linux__
        sprintf(
#endif
#if _WIN32
        sprintf_s(
#endif
            console_msg, "POSE Filtered Command Feedback: x= %f, y= %f, z= %f, a= %f, b= %f, c= %f, l1= %f, l2= %f, sflg1= %d, sflg2= %d\n",
                  fb_pack.pos_FFB.w.x,
                  fb_pack.pos_FFB.w.y,
                  fb_pack.pos_FFB.w.z,
                  fb_pack.pos_FFB.w.a,
                  fb_pack.pos_FFB.w.b,
                  fb_pack.pos_FFB.w.c,
                  fb_pack.pos_FFB.w.l1,
                  fb_pack.pos_FFB.w.l2,
                  fb_pack.pos_FFB.sflg1,
                  fb_pack.pos_FFB.sflg2);
        std::cout << console_msg;
#if __linux__
        sprintf(
#endif
#if _WIN32
        sprintf_s(
#endif
            console_msg, "POSE Encoder Feedback: x= %f, y= %f, z= %f, a= %f, b= %f, c= %f, l1= %f, l2= %f, sflg1= %d, sflg2= %d\n",
                  fb_pack.pos_EFB.w.x,
                  fb_pack.pos_EFB.w.y,
                  fb_pack.pos_EFB.w.z,
                  fb_pack.pos_EFB.w.a,
                  fb_pack.pos_EFB.w.b,
                  fb_pack.pos_EFB.w.c,
                  fb_pack.pos_EFB.w.l1,
                  fb_pack.pos_EFB.w.l2,
                  fb_pack.pos_EFB.sflg1,
                  fb_pack.pos_EFB.sflg2);
        std::cout << console_msg;
#if __linux__
        sprintf(
#endif
#if _WIN32
        sprintf_s(
#endif
            console_msg, "PULSE Command Feedback: p1= %d, p1= %d, p3= %d, p4= %d, p5= %d, p6= %d, p7= %d, p8= %d\n",
                  fb_pack.pls_FB.p1,
                  fb_pack.pls_FB.p2,
                  fb_pack.pls_FB.p3,
                  fb_pack.pls_FB.p4,
                  fb_pack.pls_FB.p5,
                  fb_pack.pls_FB.p6,
                  fb_pack.pls_FB.p7,
                  fb_pack.pls_FB.p8);
        std::cout << console_msg;
#if __linux__
        sprintf(
#endif
#if _WIN32
        sprintf_s(
#endif
            console_msg, "PULSE Filtered Command Feedback: p1= %d, p1= %d, p3= %d, p4= %d, p5= %d, p6= %d, p7= %d, p8= %d\n",
                  fb_pack.pls_FFB.p1,
                  fb_pack.pls_FFB.p2,
                  fb_pack.pls_FFB.p3,
                  fb_pack.pls_FFB.p4,
                  fb_pack.pls_FFB.p5,
                  fb_pack.pls_FFB.p6,
                  fb_pack.pls_FFB.p7,
                  fb_pack.pls_FFB.p8);
        std::cout << console_msg;
#if __linux__
        sprintf(
#endif
#if _WIN32
        sprintf_s(
#endif
            console_msg, "PULSE Encoder Feedback: p1= %d, p1= %d, p3= %d, p4= %d, p5= %d, p6= %d, p7= %d, p8= %d\n",
                  fb_pack.pls_EFB.p1,
                  fb_pack.pls_EFB.p2,
                  fb_pack.pls_EFB.p3,
                  fb_pack.pls_EFB.p4,
                  fb_pack.pls_EFB.p5,
                  fb_pack.pls_EFB.p6,
                  fb_pack.pls_EFB.p7,
                  fb_pack.pls_EFB.p8);
        std::cout << console_msg;
#if __linux__
        sprintf(
#endif
#if _WIN32
        sprintf_s(
#endif
            console_msg, "CURRENT Command Feedback: p1= %d, p1= %d, p3= %d, p4= %d, p5= %d, p6= %d, p7= %d, p8= %d\n",
                  fb_pack.curr_cmd.p1,
                  fb_pack.curr_cmd.p2,
                  fb_pack.curr_cmd.p3,
                  fb_pack.curr_cmd.p4,
                  fb_pack.curr_cmd.p5,
                  fb_pack.curr_cmd.p6,
                  fb_pack.curr_cmd.p7,
                  fb_pack.curr_cmd.p8);
        std::cout << console_msg;
#if __linux__
        sprintf(
#endif
#if _WIN32
        sprintf_s(
#endif
            console_msg, "CURRENT Actual Feedback: p1= %d, p1= %d, p3= %d, p4= %d, p5= %d, p6= %d, p7= %d, p8= %d\n",
                  fb_pack.curr_FB.p1,
                  fb_pack.curr_FB.p2,
                  fb_pack.curr_FB.p3,
                  fb_pack.curr_FB.p4,
                  fb_pack.curr_FB.p5,
                  fb_pack.curr_FB.p6,
                  fb_pack.curr_FB.p7,
                  fb_pack.curr_FB.p8);
        std::cout << console_msg;
        std::cout << "Packet Sent: " << counter_ << "\n";
        std::cout << "Packet lost: " << packet_recv_lost << "\n";
        return 0;
    }
}
