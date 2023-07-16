#pragma once

#if defined(bcm2835)

#include <stdio.h>
#include <stdint.h>
#include <bcm2835.h>

#define MSBFIRST BCM2835_SPI_BIT_ORDER_MSBFIRST
#define SPI_MODE0 BCM2835_SPI_MODE0
#define SPI_MODE1 BCM2835_SPI_MODE1
#define SPI_MODE2 BCM2835_SPI_MODE2
#define SPI_MODE3 BCM2835_SPI_MODE3

struct SPISettings;

class SPIClass
{
public:
	void beginTransaction(SPISettings settings);
	void endTransaction();
	uint8_t transfer(uint8_t);
};

struct SPISettings
{
	friend class SPIClass;
	SPISettings(uint32_t s, bcm2835SPIBitOrder o, bcm2835SPIMode m);

	uint32_t speed;
	bcm2835SPIBitOrder order;
	bcm2835SPIMode mode;
};

extern SPIClass SPI;
#endif