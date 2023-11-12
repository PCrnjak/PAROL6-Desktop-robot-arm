#ifndef IODEFS_H
#define IODEFS_H

#include <Arduino.h>
#include <stdio.h>

#define PUL1 PC6 
#define PUL2 PA10 
#define PUL3 PC0 
#define PUL4 PC3 
#define PUL5 PC9 
#define PUL6 PC5 

#define DIR1 PB15 
#define DIR2 PA1 
#define DIR3 PC1 
#define DIR4 PA0 
#define DIR5 PA8 
#define DIR6 PB1 

#define LIMIT1 PC12 
#define LIMIT2 PB3 
#define LIMIT3 PA15 
#define LIMIT4 PD2 
#define LIMIT5 PB4 
#define LIMIT6 PC11 

#define SELECT1 PC7 
#define SELECT2 PA9
#define SELECT3 PC15
#define SELECT4 PC2
#define SELECT5 PC8
#define SELECT6 PC4

#define GLOBAL_ENABLE PA3

#define MISO PA6
#define MOSI PA7
#define SCK PA5
#define FLASH_SELECT PA4

#define LED1 PB2
#define LED2 PB10

#define SUPPLY_ON_OFF PC10
#define SUPPLY_BUTTON_STATE PC14

#define INPUT1 PB6
#define INPUT2 PB5

#define OUTPUT1 PC13
#define OUTPUT2 PB7

#define ESTOP PB14

#define VBUS PB0  // ADC1 CHANNEL 8

#define USB_D_PLUS  PA12 
#define USB_D_MINUS PA11 

#define CAN1TX PB9
#define CAN1RX PB8

#define CAN1TX PB13
#define CAN1RX PB12


#endif