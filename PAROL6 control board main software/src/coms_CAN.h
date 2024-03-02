/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    coms_CAN.h
  * @brief   This file provides code for all the function prototypes for communication_CAN.cpp
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


#ifndef CAN_COMS_H
#define CAN_COMS_H

#include <TMCStepper.h>
#include <Arduino.h>
#include <AccelStepper.h>
#include <Arduino.h>
#include "hw_init.h"
#include "iodefs.h"
#include "utils.h"
#include "common.h"
#include "constants.h"
#include "adc_init.h"
#include "stm32f4xx_hal.h"
#include "motor_init.h"
#include "CAN.h"
#include "structs.h"

void Extract_from_CAN_ID(unsigned int canId);

unsigned int Combine_2_CAN_ID(unsigned int Node_ID, unsigned int Command_ID, bool errorBit);

void Setup_CAN_bus();

void CAN_protocol(Stream &Serialport);

void Send_gripper_cal();

void Send_gripper_pack();

void Send_clear_error();

void Send_gripper_pack_empty();

#endif