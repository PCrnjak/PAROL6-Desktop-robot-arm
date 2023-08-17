/** @file hw_init.cpp
    @brief A Documented file.
    
    Source file of hardware initialization.
    For schematic please refer to:
    For additional pin functionality refer to:

*/

#include "hw_init.h"
#include "iodefs.h"
#include <Arduino.h>

/// Define all ADC pins
/// Pins for DC bus measurment and termistor
void ADC_piins(void){

}

void Init_Digital_Inputs(void){

    pinMode(LIMIT1, INPUT);
    pinMode(LIMIT2, INPUT);
    pinMode(LIMIT3, INPUT);
    pinMode(LIMIT4, INPUT);
    pinMode(LIMIT5, INPUT);
    pinMode(LIMIT6, INPUT);

    pinMode(SUPPLY_BUTTON_STATE, INPUT);

    pinMode(INPUT1, INPUT);
    pinMode(INPUT2, INPUT);

    pinMode(ESTOP, INPUT);


}

void Init_Digital_Outputs(void){

    pinMode(PUL1, OUTPUT);
    pinMode(PUL2, OUTPUT);
    pinMode(PUL3, OUTPUT);
    pinMode(PUL4, OUTPUT);
    pinMode(PUL5, OUTPUT);
    pinMode(PUL6, OUTPUT);

    pinMode(DIR1, OUTPUT);
    pinMode(DIR2, OUTPUT);
    pinMode(DIR3, OUTPUT);
    pinMode(DIR4, OUTPUT);
    pinMode(DIR5, OUTPUT);
    pinMode(DIR6, OUTPUT);

    pinMode(SELECT1, OUTPUT);
    pinMode(SELECT2, OUTPUT);
    pinMode(SELECT3, OUTPUT);
    pinMode(SELECT4, OUTPUT);
    pinMode(SELECT5, OUTPUT);
    pinMode(SELECT6, OUTPUT);
    pinMode(FLASH_SELECT, OUTPUT);

    pinMode(GLOBAL_ENABLE, OUTPUT);


    pinMode(OUTPUT1, OUTPUT); 
    pinMode(OUTPUT2, OUTPUT); 

    pinMode(LED1, OUTPUT); 
    pinMode(LED2, OUTPUT); 

    pinMode(SUPPLY_ON_OFF, OUTPUT); 

}

void Init_SPI(void){

}

void Init_CAN(void){

}

void Init_False_Trigger_ISR(){
  attachInterrupt(LIMIT1, False_Swtich_Trigger_1, FALLING);
  attachInterrupt(LIMIT2, False_Swtich_Trigger_2, RISING);
  attachInterrupt(LIMIT3, False_Swtich_Trigger_3, RISING);
  attachInterrupt(LIMIT4, False_Swtich_Trigger_4, RISING);
  attachInterrupt(LIMIT5, False_Swtich_Trigger_5, RISING);
  attachInterrupt(LIMIT6, False_Swtich_Trigger_6, FALLING);
}


/// Enable false trigger ISR
void Enable_False_Trigger_ISR();
/// Disable false trigger ISR

void Disable_False_Trigger_ISR(){
  detachInterrupt(LIMIT1);
  detachInterrupt(LIMIT2);
  detachInterrupt(LIMIT3);
  detachInterrupt(LIMIT4);
  detachInterrupt(LIMIT5);
  detachInterrupt(LIMIT6);

}
/// ISR callback functions for limit switches
void False_Swtich_Trigger_1(void){
    // neki error flag digni 

}

void False_Swtich_Trigger_2(void){

}

void False_Swtich_Trigger_3(void){

}

void False_Swtich_Trigger_4(void){

}

void False_Swtich_Trigger_5(void){

}

void False_Swtich_Trigger_6(void){

}





void Init_ALL_HW(void){

    Init_Digital_Inputs();
    Init_Digital_Outputs();
    Init_ADC();
    Init_USB();
    Init_SPI();
    Init_CAN();


}
