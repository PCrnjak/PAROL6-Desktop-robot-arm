/** @file hw_init.h
    @brief A Documented file.
    
    Header file of hardware initialization.
    For schematic please refer to:
    For additional pin functionality refer to:

*/

#ifndef HW_SETUP_H
#define HW_SETUP_H

#include <Arduino.h>
#include <stdio.h>

/// Initialize all ADC pins
void Init_ADC (void);
/// Initialize USB port
void Init_USB(void);
/// Initialize digital input pins
void Init_Digital_Inputs(void);
/// Initialize digital output pins
void Init_Digital_Outputs(void);
/// Initialize SPI 
void Init_SPI(void);
/// Initialize CAN
void Init_CAN(void);
/// Initialize all hardware 
void Init_ALL_HW(void);
/// Initialize false trigger ISR
void Init_False_Trigger_ISR();
/// Enable false trigger ISR
void Enable_False_Trigger_ISR();
/// Disable false trigger ISR
void Disable_False_Trigger_ISR();
/// ISR callback functions for limit switches
void False_Swtich_Trigger_1(void);
void False_Swtich_Trigger_2(void);
void False_Swtich_Trigger_3(void);
void False_Swtich_Trigger_4(void);
void False_Swtich_Trigger_5(void);
void False_Swtich_Trigger_6(void);

#endif