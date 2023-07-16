#pragma once
#include <bcm2835.h>
#include <stdio.h>
#include <stdint.h>
#define INPUT BCM2835_GPIO_FSEL_INPT
#define INPUT_PULLUP BCM2835_GPIO_PUD_UP
#define INPUT_PULLDOWN BCM2835_GPIO_PUD_DOWN
#define OUTPUT BCM2835_GPIO_FSEL_OUTP

#define pinMode(PIN, MODE) bcm2835_gpio_fsel(PIN, MODE); if (MODE == (uint8_t)BCM2835_GPIO_PUD_UP || MODE == (uint8_t)BCM2835_GPIO_PUD_DOWN) bcm2835_gpio_set_pud(PIN, MODE)
#define digitalWrite(PIN, MODE) bcm2835_gpio_write(PIN, MODE)
#define digitalRead(PIN)  bcm2835_gpio_lev(PIN)
