#if defined(bcm2835)

#include "bcm2835_spi.h"

SPIClass SPI;

void SPIClass::beginTransaction(SPISettings settings)
{
	bcm2835_spi_begin();
	bcm2835_spi_setBitOrder(settings.order);
	bcm2835_spi_setDataMode(settings.mode);
	bcm2835_spi_set_speed_hz(settings.speed);
	bcm2835_spi_chipSelect(BCM2835_SPI_CS_NONE);
}

void SPIClass::endTransaction()
{
	bcm2835_spi_end();
}

uint8_t SPIClass::transfer(uint8_t value)
{
	return bcm2835_spi_transfer(value);
}

SPISettings::SPISettings(uint32_t s, bcm2835SPIBitOrder o, bcm2835SPIMode m)
{
	speed = s;
	order = o;
	mode = m;
}

#endif