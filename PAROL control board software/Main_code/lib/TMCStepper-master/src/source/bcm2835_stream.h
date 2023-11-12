#pragma once
#if defined(bcm2835)

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <bcm2835.h>

uint32_t millis();

class Stream
{
public:
	Stream(const char* port);
	void begin(unsigned long baud) { begin(baud, O_RDWR | O_NOCTTY | O_NDELAY); }
	void begin(unsigned long, int);
	void end();
	int available(void);
	uint8_t write(const uint8_t data);
	uint8_t read();
private:
	int fd;                    /* Filedeskriptor */
	const char* port;
};

extern Stream Serial;
extern Stream Serial1;
#endif
