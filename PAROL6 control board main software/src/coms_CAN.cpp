/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    communication_CAN.cpp
 * @brief   This file provides code for CAN protocol
 * @author Petar Crnjak
 ******************************************************************************
 * @attention
 *
 * Copyright (c) Source robotics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/

#include "coms_CAN.h"

/*
The node with the lowest ID will always win the arbitration and therefore has the highest priority.
*/
// 4 + 6 + 1; FIRST 4 MSB are ID, 6 are command ID, and last is an error bit

CAN_msg_t CAN_TX_msg;
CAN_msg_t CAN_RX_msg;

#define DEBUG_COMS 0

/// @brief Extract Node Id, Node msg and Error bit from 11bit can ID frame
/// @param canId 11 bit can ID frame
void Extract_from_CAN_ID(unsigned int canId)
{
    // Extracting ID2 (first 4 MSB)
    unsigned int ID2 = (canId >> 7) & 0xF;

    // Extracting CAN Command (next 6 bits)
    unsigned int canCommand = (canId >> 1) & 0x3F;

    // Extracting Error Bit (last bit)
    unsigned int errorBit = canId & 0x1;
}

/// @brief Combine Node_ID, Command_ID and Error into standard 11 bit CAN ID
/// @param Node_ID
/// @param Command_ID
/// @param errorBit
/// @return standard 11 bit CAN ID
unsigned int Combine_2_CAN_ID(unsigned int Node_ID, unsigned int Command_ID, bool errorBit)
{
    // Combine components into an 11-bit CAN ID
    unsigned int canId = 0;

    // Add ID2 (first 4 MSB)
    canId |= (Node_ID & 0xF) << 7;

    // Add CAN Command (next 6 bits)
    canId |= (Command_ID & 0x3F) << 1;

    // Add Error Bit (last bit)
    canId |= (errorBit & 0x1);

    return canId;
}

/// @brief  Setup CAN bus hardware
void Setup_CAN_bus()
{
    bool ret = CANInit(CAN_1000KBPS, 2);
    if (!ret)
        while (true)
            ;
}

/// @brief CAN protocol
/// @param Serialport
void CAN_protocol(Stream &Serialport)
{

    if (CANMsgAvail(1))
    {

        /// Get CAN msg from buffer
        CANReceive(1, &CAN_RX_msg);
        /// Unpack CAN ID
        unsigned int Node_ID = (CAN_RX_msg.id >> 7) & 0xF;
        unsigned int Command_ID = (CAN_RX_msg.id >> 1) & 0x3F;
        unsigned int Error_bit = CAN_RX_msg.id & 0x1;

        /// Print CAN data
        #if (DEBUG_COMS > 0)
                Serialport.print("Node ID: ");
                Serialport.println(Node_ID);
                Serialport.print("Command ID: ");
                Serialport.println(Command_ID);
                Serialport.print("Error Bit: ");
                Serialport.println(Error_bit);
                Serialport.print("Msg length: ");
                Serialport.println(CAN_RX_msg.len);
                Serialport.println("");
                Serialport.print("Is standard frame: ");
                Serialport.println(CAN_RX_msg.type);
                Serialport.println("");
        #endif

        // If node ID matches the ID of
        if (Node_ID == Comp_gripper.commanded_ID)
        {

            // Perform action depending on command ID we received FROM the gripper!
            switch (Command_ID)
            {
            case 60:
            { // Gripper data pack

                if (CAN_RX_msg.len == 4)
                {

                    /// Position 1 byte
                    /// Current 2 byte
                    /// 1 byte:
                    ///  bit 0 - gripper activate (1) / deactivated (0)
                    ///  bit 1 Gripper action status - 1 is goto, 0 is idle or performing auto release or in calibration)
                    ///  object detection bit 2 and 3 -
                    ///  bit 4 - gripper temperature error
                    ///  bit 5 - gripper timeout error
                    ///  bit 6 - gripper estop error
                    ///  bit 7 - gripper calibration status; calibrated (1) / not calibrated (0)
                    Comp_gripper.current_position = CAN_RX_msg.data[0];
                    uint8_t temp_buffer[] = {CAN_RX_msg.data[1], CAN_RX_msg.data[2]};
                    Comp_gripper.current_current = two_bytes_to_int(temp_buffer); // current setpoint
                    //bool bitArray[8];
                    //byteToBitsBigEndian(CAN_RX_msg.data[3], bitArray);
                    Comp_gripper.current_status = CAN_RX_msg.data[3];
                    #if (DEBUG_COMS > 0)
                    Serialport.print("Position is:");
                    Serialport.println(Comp_gripper.current_position);
                    Serialport.print("Current is:");
                    Serialport.println(Comp_gripper.current_current);
                    Serialport.print("Data byte is: ");
                    Serialport.println(Comp_gripper.current_status);
                    #endif

                    break;
                }
                break;
            }
            }
        }else{
            #if (DEBUG_COMS > 0)
            Serialport.print("Wrong ID");
            #endif
        }
    }
}

/// @brief Griper calib; direction: Mainboard -> Gripper 
void Send_gripper_cal()
{

    CAN_TX_msg.data[0] = 0x00;
    CAN_TX_msg.data[1] = 0x00;
    CAN_TX_msg.data[2] = 0x00;
    CAN_TX_msg.data[3] = 0x00;
    CAN_TX_msg.data[4] = 0x00;
    CAN_TX_msg.data[5] = 0x00;
    CAN_TX_msg.data[6] = 0x00;
    CAN_TX_msg.data[7] = 0x00;
    CAN_TX_msg.len = 0;
    CAN_TX_msg.type = DATA_FRAME;
    CAN_TX_msg.format = STANDARD_FORMAT;
    CAN_TX_msg.id = Combine_2_CAN_ID(Comp_gripper.commanded_ID, 62, 0);
    CANSend(1, &CAN_TX_msg);
}


/// @brief Send Gripper packet; direction: Mainboard -> Gripper 
void Send_gripper_pack()
{
    // pos setpoint 1 byte
    // speed setpoint 1 byte
    // current setpoint 2 byte
    // data 4 bits: activated, action_status, estop_status, relese_dir
    byte data_buffer_send[2];
    intTo2Bytes(Comp_gripper.commanded_current, data_buffer_send);
    bool bitArray[8];
    byteToBitsBigEndian(Comp_gripper.command, bitArray);
    CAN_TX_msg.data[0] = Comp_gripper.commanded_position;
    CAN_TX_msg.data[1] = Comp_gripper.commanded_speed;
    CAN_TX_msg.data[2] = data_buffer_send[0];
    CAN_TX_msg.data[3] = data_buffer_send[1];
    CAN_TX_msg.data[4] = bitsToByte(bitArray);
    CAN_TX_msg.data[5] = 0x00;
    CAN_TX_msg.data[6] = 0x00;
    CAN_TX_msg.data[7] = 0x00;
    CAN_TX_msg.len = 5;
    CAN_TX_msg.type = DATA_FRAME;
    CAN_TX_msg.format = STANDARD_FORMAT;
    CAN_TX_msg.id = Combine_2_CAN_ID(Comp_gripper.commanded_ID, 61, 0);
    CANSend(1, &CAN_TX_msg);
}


/// @brief Send Gripper packet empty; direction: Mainboard -> Gripper 
void Send_gripper_pack_empty()
{

    CAN_TX_msg.data[0] = 0x00;;
    CAN_TX_msg.data[1] = 0x00;
    CAN_TX_msg.data[2] = 0x00;
    CAN_TX_msg.data[3] = 0x00;
    CAN_TX_msg.data[4] = 0x00;
    CAN_TX_msg.data[5] = 0x00;
    CAN_TX_msg.data[6] = 0x00;
    CAN_TX_msg.data[7] = 0x00;
    CAN_TX_msg.len = 0;
    CAN_TX_msg.type = DATA_FRAME;
    CAN_TX_msg.format = STANDARD_FORMAT;
    CAN_TX_msg.id = Combine_2_CAN_ID(Comp_gripper.commanded_ID, 61, 0);
    CANSend(1, &CAN_TX_msg);
}



/// @brief Clear error; direction: Mainboard -> Gripper 
void Send_clear_error()
{
    CAN_TX_msg.data[0] = 0x00;
    CAN_TX_msg.data[1] = 0x00;
    CAN_TX_msg.data[2] = 0x00;
    CAN_TX_msg.data[3] = 0x00;
    CAN_TX_msg.data[4] = 0x00;
    CAN_TX_msg.data[5] = 0x00;
    CAN_TX_msg.data[6] = 0x00;
    CAN_TX_msg.data[7] = 0x00;
    CAN_TX_msg.len = 0;
    CAN_TX_msg.type = DATA_FRAME;
    CAN_TX_msg.format = STANDARD_FORMAT;
    CAN_TX_msg.id = Combine_2_CAN_ID(Comp_gripper.commanded_ID, 1, 0);
    CANSend(1, &CAN_TX_msg);
}