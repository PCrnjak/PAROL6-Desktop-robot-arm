#if defined(bcm2835)
#include <cstring>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "bcm2835_stream.h"

uint32_t millis()
{
	struct timeval now;
	gettimeofday(&now, NULL);
	return (uint32_t) ( now.tv_usec / 1000 );
}

Stream::Stream(const char* port)
{
	Stream::port = port;
}

void Stream::begin(unsigned long baud, int flags)
{
	struct termios options;

	fd = open(port, flags);
	if (fd == -1) {
		printf("[ERROR] UART open(%s)\n", port);
		return;
	}
	fcntl(fd, F_SETFL, O_RDWR);


	speed_t myBaud;
	switch ( baud )
	{
		case    9600:	myBaud =    B9600; break;
		case   19200:	myBaud =   B19200; break;
		case   38400:	myBaud =   B38400; break;
		case   57600:	myBaud =   B57600; break;
		case  115200:	myBaud =  B115200; break;
		case  230400:	myBaud =  B230400; break;
		case  460800:	myBaud =  B460800; break;
		case  500000:	myBaud =  B500000; break;
		case  576000:	myBaud =  B576000; break;

		// The TMC2209 maxed out around 750kbaud so other enums after 576kbaud is not needed.

		default:
			printf("[ERROR] UART invalid baud: %ld for port: %s\n", baud, port);
			return;
	}

	termios options{};

	tcgetattr(fd, &options);

	cfmakeraw( &options );
	cfsetispeed( &options, myBaud );
	cfsetospeed( &options, myBaud );

	// See: https://www.mkssoftware.com/docs/man5/struct_termios.5.asp
	// Use 8 data bit, no parity and 1 stop bit
	// Set bits per byte
	options.c_cflag |= ( CREAD | CLOCAL ) ;   // turn on READ and ignore modem ctrl lines
	// CBAUDEX no need to use extended baud; setting this for some reason shows incorrect timing on the oscilloscope for 9600 but OK for 57600
	// Not setting it seems to be OK on the oscilloscope for both 9600 and 57600.
	options.c_cflag &= ~PARENB;     // no parity
	options.c_cflag &= ~CSTOPB;     // 1 stop bit
	options.c_cflag &= ~CSIZE;      // reset number of bits mask
	options.c_cflag |= CS8;         // 8 data bit
	options.c_cflag &= ~CRTSCTS;   // no flow control

	options.c_iflag &= ~(IXON | IXOFF | IXANY); // turn off s/w flow ctrl
	options.c_iflag |= IGNPAR;       // ignore characters with parity errors

	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG) ;    // make raw

	options.c_oflag &= ~OPOST;  // make raw

	// See: http://unixwiz.net/techtips/termios-vmin-vtime.html
	options.c_cc[VMIN]  = 0;
	// Cannot set VTIME to 0 as it creates more problems such as cannot read the driver properly.
    // Also, you do not want to set to 100 because it is a blocking read and timeout in VTIME and can take up to 10 seconds and
    // if retries are implemented in caller then it feels like something is blocking for N retries x VTIME.
    options.c_cc[ VTIME ] = 10;      // VTIME defined as tenths of a second so 100 is actually 10 seconds; and 10 deciseconds is 1 second.

	tcflush(fd, TCIOFLUSH); // flush both tx and rx
	tcsetattr(fd, TCSANOW, &options);

	// Maybe add 10ms delay (belt and braces) to let UART setup correctly
	const int DELAY_MS_10 = 10;
	usleep( 1000 * DELAY_MS_10 );
}

void Stream::end()
{
	::close(fd);
}

int Stream::available()
{
	int result;
	if (ioctl(fd, FIONREAD, &result) == -1)
		return -1;
	return result;
}

uint8_t Stream::write(const uint8_t data)
{
    return (uint8_t)::write(fd, &data, 1);
}

uint8_t Stream::read()
{
	uint8_t data = -1;
	if (::read(fd, &data, 1) == -1)
		return -1;
	return data;
}

Stream Serial("/dev/serial0");
Stream Serial1("/dev/serial1");

#endif
