#pragma once
#if defined(ARDUINO) && ARDUINO >= 100
	#include <Arduino.h>
#elif defined(bcm2835)
	#include "source/rpi_bcm2835.h"
	#include "source/bcm2835_spi.h"
	#include "source/bcm2835_stream.h"
#elif __cplusplus >= 201703L
	#if __has_include(<Arduino.h>)
		#include <Arduino.h>
	#endif
#endif

#include "TMC_platforms.h"

class SSwitch {
  public:
    SSwitch(const uint16_t pin1, const uint16_t pin2, const uint8_t address);
    void active();
  private:
    const uint16_t p1;
    const uint16_t p2;
    const uint8_t addr;
};
