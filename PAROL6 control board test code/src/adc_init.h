/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    adc.h
  * @brief   This file contains all the function prototypes for
  *          the adc.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ADC_H__
#define __ADC_H__

#ifdef __cplusplus
extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

extern ADC_HandleTypeDef hadc1;

void MX_ADC1_Init(void);

int ADC_CHANNEL_8_READ_BUS_VOLTAGE(void);

void HAL_ADC_MspInit_(ADC_HandleTypeDef* adcHandle);

int BUS_voltage(void);

#ifdef __cplusplus
}
#endif

#endif