/** @file utils.h
    @brief A Documented file.
    
    Header file for utilities functions
    For schematic please refer to:
    For additional pin functionality refer to:

*/

#ifndef UTILS_
#define UTILS_

#include <Arduino.h>
#include <stdio.h>
#include "iodefs.h"
#include "hw_init.h"
#include <SPI.h>
#include "constants.h"

//extern volatile uint32_t counter_1us = 0;

int JOINT(int joint__);
void Ticker_init(TIM_TypeDef *Instance, int frequency,  void (*int_callback)());
void Turn_on_24V(void);
void Turn_off_24V(void);
void Power_switch_managment();
void Init_tick_1us();
void tick_1us();
uint32_t us_tick();
void intTo3Bytes(int32_t value, byte *bytes);
void intTo2Bytes(int32_t value, byte *bytes);
int bytes_to_int(uint8_t *bytes);
int two_bytes_to_int(uint8_t *bytes);
unsigned char bitsToByte(const bool *bits);
void byteToBits(byte b, bool* bits);
void byteToBitsBigEndian(byte b, bool* bits);

#endif
