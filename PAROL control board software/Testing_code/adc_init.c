/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    adc.c
  * @brief   This file provides code for the configuration
  *          of the ADC instances.
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
/* Includes ------------------------------------------------------------------*/

#include "adc_init.h"

ADC_HandleTypeDef hadc1;

/* ADC1 init function */
void MX_ADC1_Init(void)
{

  /* Enable clock of ADCx peripheral */
  __HAL_RCC_ADC1_CLK_ENABLE();

  /* Configure ADCx clock prescaler */
  /* Caution: On STM32F1, ADC clock frequency max is 14MHz (refer to device   */
  /*          datasheet).                                                     */
  /*          Therefore, ADC clock prescaler must be configured in function   */
  /*          of ADC clock source frequency to remain below this maximum      */
  /*          frequency.  

  /*                                               
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

  */
  ADC_ChannelConfTypeDef sConfig = {0};

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 3;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.NbrOfDiscConversion = 3;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;

  HAL_ADC_Init(&hadc1);
  

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_8;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES; ///ADC_SAMPLETIME_7CYCLES_5 ///ADC_SAMPLETIME_15CYCLES
  HAL_ADC_ConfigChannel(&hadc1, &sConfig);
  
}

void HAL_ADC_MspInit_(ADC_HandleTypeDef* adcHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(adcHandle->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspInit 0 */

  /* USER CODE END ADC1_MspInit 0 */
    /* ADC1 clock enable */
    __HAL_RCC_ADC1_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**ADC1 GPIO Configuration
    PB0     ------> ADC1_IN8
    */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN ADC1_MspInit 1 */

  /* USER CODE END ADC1_MspInit 1 */
  }
}

/** 
 * Calculates voltage of our DC supply voltage.
 * Returns voltage in ADC ticks
 * @param[out] v_
*/
int ADC_CHANNEL_8_READ_BUS_VOLTAGE(void){

  HAL_ADC_Start(&hadc1);
  // Poll ADC1 Perihperal & TimeOut = 1mSec
  HAL_ADC_PollForConversion(&hadc1, 1);
  // Read The ADC Conversion Result & Map It To PWM DutyCycle
 int AD_RES = HAL_ADC_GetValue(&hadc1);
 return AD_RES;

} 

/** 
 * Calculates voltage of our DC supply voltage.
 * Returns voltage in mv
 * Resistors are 110k and 16K
 * @param[out] v_
*/
int BUS_voltage(void){

int R1 = 110000; // 110k ohms
int R2 = 16000;  // 16k ohms


float referenceVoltage = 3.3; // 3.3V
int adcResolution = 4095;      // 12-bit ADC, so 2^12 - 1

int adcValue  = ADC_CHANNEL_8_READ_BUS_VOLTAGE();

// Calculate the Voltage Divider Ratio
float voltageDividerRatio = (float)R2 / (R1 + R2); // Cast to float for floating-point division

// Calculate the measured voltage in millivolts
int measuredVoltage = (int)(adcValue * (referenceVoltage * 1000.0f) / adcResolution / voltageDividerRatio);

return measuredVoltage;
}